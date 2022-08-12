#include <stdio.h>
#include <stdlib.h>

/* spl - https://github.com/mrsafalpiya/spl */
#include "dep/spl/spl_flags.h"
#define SPLU_IMPL
#include "dep/spl/spl_utils.h"

/* spnotes - https://github.com/mrsafalpiya/spnotes */
#define SPNOTES_IMPL
#include "dep/spnotes.h"

/* config file */
#include "config.h"

/*
 ===============================================================================
 |                                   Macros                                    |
 ===============================================================================
 */

#define USAGE_STR                                                                                                              \
	"Usage: %s [(a)dd/(r)emove/(l)ist/(p)ath] [(c)ategory/(n)ote] [categ_title] [note_title]\n\nAvailable options are:\n", \
		argv[0]

#define ERR_MORE_INFO(msg) splu_die("ERROR: " msg " Use --help for more info.");
#define ERR_ERRNO(msg)     splu_die("ERROR: " msg ": %s.", strerror(errno));
#define ERR(msg)           splu_die("ERROR: " msg ".");

/*
 ===============================================================================
 |                              Global Variables                               |
 ===============================================================================
 */

static splf_info f_info;
static spnotes_t spn_instance;
static char     *delimiter = " --- ";

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */

/* Fill all the categories as well as all the notes in each categories. */
static void
fill_categs_notes(void);

/* Print all the categories and notes in a tree view. */
static void
print_notes_tree(void);

/* Sometimes the user provides more options than anticipated and are ignored.
 * Warn about it. */
static void
warn_ignored_options(int option_index);

/*
 ===============================================================================
 |                          Function Implementations                           |
 ===============================================================================
 */

static void
fill_categs_notes(void)
{
	/* read categs */
	if (spnotes_categs_fill(&spn_instance) < 0)
		splu_die("ERROR: Couldn't get the categories: %s.",
		         spnotes_errorstr());

	/* sort according to last modified */
	spnotes_categs_sort_last_modified(&spn_instance);

	/* read notes */
	for (size_t i = 0; i < spn_instance.categs_c; i++)
		if (spnotes_notes_fill(spn_instance.categs + i) < 0)
			splu_die("ERROR: Couldn't get the notes: %s.",
			         spnotes_errorstr());

	/* sort all notes according to last modified */
	for (size_t i = 0; i < spn_instance.categs_c; i++)
		spnotes_notes_sort_last_modified(spn_instance.categs + i);
}

static void
print_notes_tree(void)
{
	for (size_t i = 0; i < spn_instance.categs_c; i++) {
		printf("%s\n", spn_instance.categs[i].title);
		for (size_t j = 0; j < spn_instance.categs[i].notes_c; j++) {
			printf("%s %s",
			       j == spn_instance.categs[i].notes_c - 1 ? "└──" :
                                                                         "├──",
			       spn_instance.categs[i].notes[j].title);
			if (spn_instance.categs[i].notes[j].has_description)
				printf("%s%s", delimiter,
				       spn_instance.categs[i]
				               .notes[j]
				               .description);
			printf("\n");
		}
	}
}

static void
warn_ignored_options(int option_index)
{
	if (option_index >= f_info.non_flag_arguments_c) {
		return;
	}
	fprintf(stderr, "WARNING: Following options are ignored: ");

	for (int i = option_index; i < f_info.non_flag_arguments_c; i++)
		fprintf(stderr, "\"%s\"%c", f_info.non_flag_arguments[i],
		        i == f_info.non_flag_arguments_c - 1 ? '.' : ' ');
	fprintf(stderr, "\n");
}

int
main(int argc, char **argv)
{
	/* flags */
	int to_print_help     = 0;
	int to_output_verbose = 0;

	splf_toggle(&to_print_help, 'h', "help", "Print help message");
	splf_toggle(&to_output_verbose, 'v', "verbose", "Verbose output");
	splf_str(&notes_root_loc, 'p', "path", "Path to the notes");
	splf_str(&delimiter, 'd', "delimiter", "Delimiter");

	f_info = splf_parse(argc, argv);

	if (to_print_help) {
		fprintf(stdout, USAGE_STR);
		splf_print_help(stdout);
	}

	/* Printing any gotchas in parsing */
	if (splf_print_gotchas(f_info, stderr))
		exit(EXIT_FAILURE);

	/* spnotes */
	if (!notes_root_loc)
		splu_die(
			"Path to the notes isn't provided. Pass one using --path.");
	spnotes_init(&spn_instance, notes_root_loc);
	fill_categs_notes();

	/* parse options */
	char *option       = *(f_info.non_flag_arguments);
	char *option_sub   = *(f_info.non_flag_arguments + 1);
	char *option_categ = *(f_info.non_flag_arguments + 2);
	char *option_note  = *(f_info.non_flag_arguments + 3);
	char *option_desc  = *(f_info.non_flag_arguments + 4);

	/* simply print the notes in tree view if no option is provided */
	if (!option) {
		print_notes_tree();
		exit(EXIT_SUCCESS);
	}

	/* actual options parsing */

	/* 'add' */
	if (!strcmp(option, "add") || !strcmp(option, "a")) {
		if (!option_sub)
			ERR_MORE_INFO("What do you want to add?");

		if (!strcmp(option_sub, "category") ||
		    !strcmp(option_sub, "c")) {
			if (!option_categ)
				ERR_MORE_INFO("Missing title of the category.");
			warn_ignored_options(3);

			/* actual adding of category */
			char new_loc[PATH_MAX];
			if (!spnotes_categs_add(spn_instance, option_categ,
			                        new_loc)) {
				switch (spnotes_err) {
				case SPNOTES_ERR_REDECLARE:
					splu_die(
						"ERROR: Category with title '%s' already exists.",
						option_categ);
				case SPNOTES_ERR_MKDIR:
					ERR_ERRNO(
						"Couldn't create a directory for new category");
				}
			}
			if (to_output_verbose)
				printf("Category '%s' added at '%s'.\n",
				       option_categ, new_loc);
			else
				printf("%s\n", new_loc);

			exit(EXIT_SUCCESS);
		}
		if (!strcmp(option_sub, "note") || !strcmp(option_sub, "n")) {
			if (!option_categ)
				ERR_MORE_INFO("Missing title of the category.");
			if (!option_note)
				ERR_MORE_INFO("Missing title of the note.");
			warn_ignored_options(5);

			/* actual adding of note */
			spnotes_categ *found_categ = spnotes_categs_search(
				spn_instance, option_categ);
			if (!found_categ)
				splu_die(
					"ERROR: Category with title '%s' doesn't exist.",
					option_categ);
			if (spnotes_notes_search(*found_categ, option_note))
				splu_die(
					"ERROR: Note with title '%s' in the category '%s' already exists.",
					option_note, option_categ);

			char new_loc[PATH_MAX];
			if (!spnotes_notes_add(*found_categ, new_loc))
				ERR_ERRNO(
					"Couldn't create a file for new note");

			/* note template */
			FILE *fp = fopen(new_loc, "w");
			if (!fp)
				ERR_ERRNO("Couldn't open note file");
			if (option_desc)
				fprintf(fp, NEW_NOTE_TEMPLATE);
			else
				fprintf(fp, NEW_NOTE_TEMPLATE_TITLE_ONLY);
			fclose(fp);

			if (to_output_verbose)
				printf("Note titled '%s' added to the category '%s' at '%s'.\n",
				       option_note, option_categ, new_loc);
			else
				printf("%s\n", new_loc);

			exit(EXIT_SUCCESS);
		}

		ERR_MORE_INFO("You can only add a category or note.");
	}

	/* 'remove' */
	if (!strcmp(option, "remove") || !strcmp(option, "r")) {
		if (!option_sub)
			ERR_MORE_INFO("What do you want to remove?");

		if (!strcmp(option_sub, "category") ||
		    !strcmp(option_sub, "c")) {
			if (!option_categ)
				ERR_MORE_INFO("Missing title of the category.");
			warn_ignored_options(3);

			/* actual removing of category */
			spnotes_categ *found_categ = spnotes_categs_search(
				spn_instance, option_categ);
			if (!found_categ)
				splu_die(
					"Category with title '%s' doesn't exist.",
					option_categ);

			/* confirm deletion if there are notes */
			if (found_categ->notes_c > 0) {
				printf("The category contains %ld note(s): ",
				       found_categ->notes_c);
				for (size_t i = 0; i < found_categ->notes_c;
				     i++)
					printf("\"%s\"%c",
					       found_categ->notes[i].title,
					       i == found_categ->notes_c - 1 ?
					               '.' :
                                                       ' ');
				printf("\nRemoving the category will remove all the above notes too! Do you want to continue? (y/n): ");
				if (getchar() != 'y')
					exit(EXIT_SUCCESS);
			}

			if (!spnotes_categs_remove(*found_categ))
				ERR_ERRNO(
					"Category directory couldn't be deleted");

			printf("Category '%s' removed.\n", option_categ);

			exit(EXIT_SUCCESS);
		}
		if (!strcmp(option_sub, "note") || !strcmp(option_sub, "n")) {
			if (!option_categ)
				ERR_MORE_INFO("Missing title of the category.");
			if (!option_note)
				ERR_MORE_INFO("Missing title of the note.");
			warn_ignored_options(4);

			/* actual removing of note */
			spnotes_categ *found_categ = spnotes_categs_search(
				spn_instance, option_categ);
			if (!found_categ)
				splu_die(
					"ERROR: Category with title '%s' doesn't exist.",
					option_categ);
			spnotes_note *found_note =
				spnotes_notes_search(*found_categ, option_note);
			if (!found_note)
				splu_die(
					"ERROR: Note with title '%s' in the category '%s' already exists.",
					option_note, option_categ);

			if (!spnotes_notes_remove(*found_note))
				ERR_ERRNO("Couldn't delete the note file");

			printf("Note titled '%s' of the category '%s' removed.\n",
			       option_note, option_categ);

			exit(EXIT_SUCCESS);
		}

		ERR_MORE_INFO("You can only delete a category or note.");
	}

	ERR_MORE_INFO("Invalid option provided.");

	return EXIT_SUCCESS;
}