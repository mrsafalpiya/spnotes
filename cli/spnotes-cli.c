#ifndef __OpenBSD__
#define _POSIX_C_SOURCE 200809L /* strdup() and strndup() */
#define _DEFAULT_SOURCE         /* d_type macro constants */
#define _XOPEN_SOURCE   500     /* nftw() */
#endif
#include <stdio.h>
#include <stdlib.h>

/* spl - https://github.com/mrsafalpiya/spl */
#include "dep/spl/spl_flags.h"
#define SPLU_IMPL
#include "dep/spl/spl_utils.h"

/* spnotes - https://github.com/mrsafalpiya/spnotes */
#define SPNOTES_IMPL
#include "../spnotes.h"

/* config file */
#include "config.h"

/*
 ===============================================================================
 |                                   Macros                                    |
 ===============================================================================
 */

#define USAGE_STR                                                                                                                     \
	"Usage: %s [(a)dd/(r)emove/(l)ist/(p)ath/(i)nfo] [(c)ategory/(n)ote] [categ_title] [note_title]\n\nAvailable options are:\n", \
		argv[0]

#define ERR_MORE_INFO(msg) splu_die("ERROR: " msg " Use --help for more info.");
#define ERR_ERRNO(msg)     splu_die("ERROR: " msg ": %s.", strerror(errno));
#define ERR(msg)           splu_die("ERROR: " msg ".");

/*
 * Why bother free'ing memory if you are going to exit anyway.
 *
 * Included only to make valgrind happy.
 */
#ifndef RELEASE
#define exit(CODE) spnotes_free(&spn_instance); exit(CODE)
#endif

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

/* Print all the categories in a list. */
static void
print_categs_list(void);

/* Print all the notes of a category in a list. */
static void
print_notes_list(spnotes_categ *categ);

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
print_categs_list(void)
{
	for (size_t i = 0; i < spn_instance.categs_c; i++)
		printf("%s\n", spn_instance.categs[i].title);
}

static void
print_notes_list(spnotes_categ *categ)
{
	for (size_t i = 0; i < categ->notes_c; i++) {
		printf("%s", categ->notes[i].title);
		if (categ->notes[i].has_description)
			printf("%s%s", delimiter, categ->notes[i].description);
		printf("\n");
	}
}

int
main(int argc, char **argv)
{
	/* flags */
	int to_print_help     = 0;
	int to_print_version  = 0;
	int to_output_verbose = 0;

	splf_toggle(&to_print_help, 'h', "help", "Print help message");
	splf_toggle(&to_print_version, 'v', "version", "Print version");
	splf_toggle(&to_output_verbose, ' ', "verbose", "Verbose output");
	splf_str(&notes_root_loc, 'p', "path", "Path to the notes");
	splf_str(&delimiter, 'd', "delimiter", "Delimiter");

	f_info = splf_parse(argc, argv);

	/* print help or version message */
	if (to_print_help) {
		printf(USAGE_STR);
		splf_print_help(stdout);
		exit(EXIT_SUCCESS);
	}
	if (to_print_version) {
		printf("spnotes-" VERSION "\n");
		exit(EXIT_SUCCESS);
	}

	/* Printing any gotchas in parsing */
	if (splf_print_gotchas(f_info, stderr)) {
		exit(EXIT_FAILURE);
	}

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
			splf_warn_ignored_args(f_info, stderr, 3);

			/* actual adding of category */
			char new_loc[PATH_MAX];
			if (!spnotes_categs_add(spn_instance, option_categ,
			                        new_loc)) {
				switch (spnotes_err) {
				case SPNOTES_ERR_REDECLARE:
					splu_die(
						"ERROR: Category with title '%s' already exists.",
						option_categ);
					break;
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
			splf_warn_ignored_args(f_info, stderr, 5);

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
			splf_warn_ignored_args(f_info, stderr, 3);

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
				if (getchar() != 'y') {
					exit(EXIT_SUCCESS);
				}
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
			splf_warn_ignored_args(f_info, stderr, 4);

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

	/* list */
	if (!strcmp(option, "list") || !strcmp(option, "l")) {
		if (!option_sub)
			ERR_MORE_INFO("What do you want to list?");

		if (!strcmp(option_sub, "category") ||
		    !strcmp(option_sub, "c")) {
			splf_warn_ignored_args(f_info, stderr, 2);

			/* list categories */
			print_categs_list();

			exit(EXIT_SUCCESS);
		}
		if (!strcmp(option_sub, "note") || !strcmp(option_sub, "n")) {
			if (!option_categ)
				ERR_MORE_INFO("Missing title of the category.");
			splf_warn_ignored_args(f_info, stderr, 3);

			/* list notes */
			spnotes_categ *found_categ = spnotes_categs_search(
				spn_instance, option_categ);
			if (!found_categ)
				splu_die(
					"ERROR: Category with title '%s' doesn't exist.",
					option_categ);
			print_notes_list(found_categ);

			exit(EXIT_SUCCESS);
		}

		ERR_MORE_INFO("You can only delete a category or note.");
	}

	/* path */
	if (!strcmp(option, "path") || !strcmp(option, "p")) {
		if (!option_sub)
			ERR_MORE_INFO("What path do you want to get?");

		if (!strcmp(option_sub, "category") ||
		    !strcmp(option_sub, "c")) {
			if (!option_categ)
				ERR_MORE_INFO("Missing title of the category.");
			splf_warn_ignored_args(f_info, stderr, 3);

			/* path of category */
			spnotes_categ *found_categ = spnotes_categs_search(
				spn_instance, option_categ);
			if (!found_categ)
				splu_die(
					"Category with title '%s' doesn't exist.",
					option_categ);

			if (to_output_verbose)
				printf("Path of the category '%s' is '%s'.\n",
				       option_categ, found_categ->path);
			else
				printf("%s\n", found_categ->path);

			exit(EXIT_SUCCESS);
		}
		if (!strcmp(option_sub, "note") || !strcmp(option_sub, "n")) {
			if (!option_categ)
				ERR_MORE_INFO("Missing title of the category.");
			if (!option_note)
				ERR_MORE_INFO("Missing title of the note.");
			splf_warn_ignored_args(f_info, stderr, 4);

			/* path of note */
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
					"ERROR: Note with title '%s' in the category '%s' doesn't exist.",
					option_note, option_categ);

			if (to_output_verbose)
				printf("Path of the note titled '%s' of category '%s' is '%s'.\n",
				       option_note, option_categ,
				       found_note->path);
			else
				printf("%s\n", found_note->path);

			exit(EXIT_SUCCESS);
		}

		ERR_MORE_INFO("You can get path of a category or note.");
	}

	/* info */
	if (!strcmp(option, "info") || !strcmp(option, "i")) {
		if (!option_sub)
			ERR_MORE_INFO("What info do you want to get?");

		if (!strcmp(option_sub, "category") ||
		    !strcmp(option_sub, "c")) {
			if (!option_categ)
				ERR_MORE_INFO("Missing title of the category.");
			splf_warn_ignored_args(f_info, stderr, 3);

			/* path of category */
			spnotes_categ *found_categ = spnotes_categs_search(
				spn_instance, option_categ);
			if (!found_categ)
				splu_die(
					"Category with title '%s' doesn't exist.",
					option_categ);

			char       time_formatted[80];
			struct tm *ts;
			ts = localtime(&found_categ->last_modified.tv_sec);
			strftime(time_formatted, sizeof(time_formatted),
			         "%a %Y-%m-%d %H:%M:%S %Z", ts);
			printf("Title: %s\nPath: %s\nLast modified: %s\nNumber of notes: %ld\nNotes: ",
			       found_categ->title, found_categ->path,
			       time_formatted, found_categ->notes_c);
			for (size_t i = 0; i < found_categ->notes_c; i++) {
				printf("'%s'", found_categ->notes[i].title);
				if (i != found_categ->notes_c)
					printf(", ");
			}
			printf("\n");

			exit(EXIT_SUCCESS);
		}
		if (!strcmp(option_sub, "note") || !strcmp(option_sub, "n")) {
			if (!option_categ)
				ERR_MORE_INFO("Missing title of the category.");
			if (!option_note)
				ERR_MORE_INFO("Missing title of the note.");
			splf_warn_ignored_args(f_info, stderr, 4);

			/* path of note */
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
					"ERROR: Note with title '%s' in the category '%s' does exist.",
					option_note, option_categ);

			char       time_formatted[80];
			struct tm *ts;
			ts = localtime(&found_note->last_modified.tv_sec);
			strftime(time_formatted, sizeof(time_formatted),
			         "%a %Y-%m-%d %H:%M:%S %Z", ts);
			printf("Title: %s\nPath: %s\nLast modified: %s\nCategory: %s\n",
			       found_note->title, found_note->path,
			       time_formatted, found_note->categ->title);

			exit(EXIT_SUCCESS);
		}

		ERR_MORE_INFO("You can get path of a category or note.");
	}

	ERR_MORE_INFO("Invalid option provided.");

	return EXIT_SUCCESS;
}
