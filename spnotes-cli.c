#define _POSIX_C_SOURCE 200809L /* strdup() and strndup() */
#define _DEFAULT_SOURCE         /* d_type macro constants */

#include <stdio.h>
#include <stdlib.h>

/* tiny-expr-c - https://github.com/kokke/tiny-regex-c */
#include "dep/tiny-regex-c/re.h"

/* spnotes - https://github.com/mrsafalpiya/spnotes */
#define SPNOTES_IMPL
#include "dep/spnotes.h"

/* spl - https://github.com/mrsafalpiya/spl */
#define SPLU_IMPL
#include "dep/spl/spl_utils.h"

#include "dep/spl/spl_flags.h"

/*
 ===============================================================================
 |                                    About                                    |
 ===============================================================================
 *
 * A simple cli interface to spnotes library.
 *
 * All this program does is list out categories, title and description of notes.
 *
 * Run `./spnotes-cli --help` to learn more.
 */

/*
 ===============================================================================
 |                              Global Variables                               |
 ===============================================================================
 */

static char *regex_categ = NULL; /* category title regex search flag */
static re_t  categ_re_t;
static char *regex_note = NULL; /* note title regex search flag */
static re_t  note_re_t;
static int   to_list_notes_only = 0;     /* list the notes only */
static char *delimiter          = "---"; /* list delimiter */

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */

/*
 * Initialize the spnotes instance as well as fill categories and notes in each
 * categories.
 *
 * Returns 0 on failure.
 */
static int
notes_init_fill(spnotes_t *instance, char *root_path);

/* Print the spnotes instance listing out the categories and notes in a tree
 * view. */
static void
notes_print_tree(spnotes_t *instance);

/* Print the spnotes instance listing out the notes along with the category in a
 * simple list. */
static void
notes_print_list(spnotes_t *instance);

/*
 ===============================================================================
 |                          Function Implementations                           |
 ===============================================================================
 */

static int
search_filter_categ(const char *whole_text, const char *search_text)
{
	(void)search_text;

	int matches;

	re_matchp(categ_re_t, whole_text, &matches);
	return matches;
}

static int
search_filter_note(const char *whole_text, const char *search_text)
{
	(void)search_text;

	int matches;

	re_matchp(note_re_t, whole_text, &matches);
	return matches;
}

static int
notes_init_fill(spnotes_t *instance, char *root_path)
{
	spnotes_init(instance, root_path);

	/* read categs */
	if (regex_categ != NULL)
		categ_re_t = re_compile(regex_categ);
	if (spnotes_categs_fill_filter(instance, regex_categ,
	                               search_filter_categ) < 0)
		return 0;

	/* sort according to last modified */
	spnotes_categs_sort_last_modified(instance);

	/* read notes */
	if (regex_note != NULL)
		note_re_t = re_compile(regex_note);
	for (size_t i = 0; i < instance->categs_c; i++)
		if (spnotes_notes_fill_filter(&(instance->categs[i]),
		                              regex_note,
		                              search_filter_note) < 0)
			return 0;

	/* sort all notes according to last modified */
	for (size_t i = 0; i < instance->categs_c; i++)
		spnotes_notes_sort_last_modified(&(instance->categs[i]));

	return 1;
}

static void
notes_print_tree(spnotes_t *instance)
{
	for (size_t i = 0; i < instance->categs_c; i++) {
		printf("%s\n", instance->categs[i].title);

		for (size_t j = 0; j < instance->categs[i].notes_c; j++) {
			printf("%s── %s",
			       j == instance->categs[i].notes_c - 1 ? "└" : "├",
			       instance->categs[i].notes[j].title);
			if (instance->categs[i].notes[j].has_description)
				printf(" %s %s", delimiter,
				       instance->categs[i].notes[j].description);
			printf("\n");
		}
	}
}

static void
notes_print_list(spnotes_t *instance)
{
	for (size_t i = 0; i < instance->categs_c; i++) {
		for (size_t j = 0; j < instance->categs[i].notes_c; j++) {
			printf("%s %s %s %s", instance->categs[i].title,
			       delimiter, instance->categs[i].notes[j].title,
			       delimiter);
			if (instance->categs[i].notes[j].has_description)
				printf(" %s",
				       instance->categs[i].notes[j].description);
			printf("\n");
		}
	}
}

int
main(int argc, char **argv)
{
	int to_print_help = 0;

	/* flags */
	splf_str(&regex_categ, 'c', "category",
	         "Perform the given regex search on category title");
	splf_str(&regex_note, 'n', "note",
	         "Perform the given regex search on note title");
	splf_str(&delimiter, 'd', "delimiter", "List delimiter");
	splf_toggle(&to_list_notes_only, 'l', "list-notes",
	            "List the notes only");
	splf_toggle(&to_print_help, 'h', "help", "Print help");

	splf_info f_info = splf_parse(argc, argv);

	/* Printing any gotchas in parsing */
	splf_print_gotchas(f_info, stderr);

	/* notes root location */
	char *notes_root_loc = NULL;
	if (f_info.non_flag_arguments_c == 0)
		splu_die("Usage: %s notes_root_loc", argv[0]);
	notes_root_loc = f_info.non_flag_arguments[0];

	/* Check if more than one non-flag arguments are passed */
	if (f_info.non_flag_arguments_c > 1) {
		fprintf(stderr, "Following non-flag arguments are ignored: ");
		for (int i = 1; i < f_info.non_flag_arguments_c; i++)
			fprintf(stderr, "%s ", f_info.non_flag_arguments[i]);
		fprintf(stderr, "\n");
	}

	/* Check if -h/--help flag was passed */
	if (to_print_help) {
		printf("Available options are:\n");
		splf_print_help(stdout);
		exit(EXIT_SUCCESS);
	}

	/* = MAIN PART = */
	/* spnotes instance */
	spnotes_t spnotes_instance;

	if (!notes_init_fill(&spnotes_instance, notes_root_loc))
		splu_die("Error: %s.", spnotes_errorstr());

	if (to_list_notes_only)
		notes_print_list(&spnotes_instance);
	else
		notes_print_tree(&spnotes_instance);

	/* clean and exit */
	spnotes_free(&spnotes_instance);
	return EXIT_SUCCESS;
}
