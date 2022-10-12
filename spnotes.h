/*
 ===============================================================================
 |                                  spnotes.h                                  |
 |                   https://github.com/mrsafalpiya/spnotes                    |
 |                                                                             |
 |                         Simple note taking library                          |
 |                                                                             |
 |                  No warranty implied; Use at your own risk                  |
 |                  See end of file for license information.                   |
 ===============================================================================
 */

/*
 ===============================================================================
 |                               Version History                               |
 ===============================================================================
 *
 - v0.2 (Current)
     - Fixed memory errors found from valgrind.
 - v0.1
 */

/*
 ===============================================================================
 |                              spnotes - layout                               |
 ===============================================================================
 *
 * The note structure of spnotes is described below:
 *
 * Basic tree structure:

$ pwd
/home/safal/notes
$ tree
.
├── binary-digits
│   └── 1648221020.md
├── c
│   ├── 1648296557.md
│   └── 1648362649.md
└── latex
    ├── 1650081313.md
    └── 1650081524.md

3 directories, 5 files

 * Here, 'binary-digits', 'c' and 'latex' are so called `categories` of the
 * system and each md file inside represents an individual note within the
 * category. Any file starting with a '.' is ignored. Only those files ending
 * with a '.md' is regarded as a note.
 *
 * All notes should have the following structure:

$ cat c/1648362649.md
---
title: Pipes
description: About pipes in C
---

# Bi-directional pipe

src: https://youtu.be/8AXEHrQTf3I
$ cat c/1648296557.md
---
title: `static inline` keyword
description: Usage of `static inline` keyword in C
---

# The actual use of `static inline`

Ref: https://youtu.be/sJuA5OPvABM

 * Note the first line of the file starting with '---' following a title,
 * description and ending with '---'. The description is optional but other
 * components has to be present in the file to be regarded as a note.
 */

/*
 ===============================================================================
 |                              spnotes - library                              |
 ===============================================================================
 *
 * This library can do all required implementations on categories/notes --
 * list, create and delete.
 */

/*
 ===============================================================================
 |                                    Usage                                    |
 ===============================================================================
 *
 * Do this:
 *
 *         #define SPNOTES_IMPL
 *
 * before you include this file in *one* C or C++ file to create the
 * implementation.
 */

/*
 ===============================================================================
 |                                Dependencies                                 |
 ===============================================================================
 *
 * - Computer with a C99 compliant C compiler.
 * - Linux requires following #define's:
 *     - #define _POSIX_C_SOURCE 200809L (for strdup() and strndup())
 *     - #define _DEFAULT_SOURCE         (for d_type macro constants)
 *     - #define _XOPEN_SOURCE   500     (for nftw())
 */

/*
 ===============================================================================
 |                              HEADER-FILE MODE                               |
 ===============================================================================
 */

#ifndef SPNOTES_H
#define SPNOTES_H

#include <string.h>
#ifdef __linux__
#include <linux/limits.h> /* NAME_MAX */
#else
#include <limits.h> /* NAME_MAX */
#endif
#include <dirent.h>
#ifdef __OpenBSD__ /* TODO: Learn more about this */
#define DT_DIR 4
#endif
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h> /* stat(), DEFFILEMODE */
#ifndef DEFFILEMODE   /* TODO: Learn more about this */
#define DEFFILEMODE 0666
#endif
#include <fcntl.h>  /* open() */
#include <unistd.h> /* close() */
#include <time.h>   /* time() */
#include <ftw.h>    /* nftw() */

/*
 ===============================================================================
 |                                   Options                                   |
 ===============================================================================
 */

/* = SPNOTES = */
#ifndef SPNOTES_DEF
#define SPNOTES_DEF /* You may want `static` or `static inline` here */
#endif

/*
 ===============================================================================
 |                                    Data                                     |
 ===============================================================================
 */

typedef struct spnotes_t     spnotes_t;
typedef struct spnotes_categ spnotes_categ;
typedef struct spnotes_note  spnotes_note;

struct spnotes_t {
	char          *root_location;
	spnotes_categ *categs; /* NULL = Not filled yet */
	size_t         categs_c;
};

struct spnotes_categ {
	spnotes_t      *spnotes_instance;
	char            path[PATH_MAX];
	char            title[NAME_MAX];
	struct timespec last_modified;
	spnotes_note   *notes; /* NULL = Not filled yet */
	size_t          notes_c;
};

struct spnotes_note {
	char            path[PATH_MAX];
	char            title[NAME_MAX];
	char           *description; /* dynamically allocated */
	int             has_description;
	struct timespec last_modified;
	spnotes_categ  *categ;
};

/*
 ===============================================================================
 |                              Global Variables                               |
 ===============================================================================
 */

static int spnotes_err;

/* error values */
#define SPNOTES_ERR_NONE        0
#define SPNOTES_ERR_NULL_PTR    1
#define SPNOTES_ERR_MALLOC      2
#define SPNOTES_ERR_REALLOC     3
#define SPNOTES_ERR_DIR_READ    4
#define SPNOTES_ERR_INVALID_LOC 5
#define SPNOTES_ERR_FILE_READ   6
#define SPNOTES_ERR_FILE_STAT   7
#define SPNOTES_ERR_REDECLARE   8
#define SPNOTES_ERR_NOT_FILLED  9
#define SPNOTES_ERR_MKDIR       10 /* errno is set */
#define SPNOTES_ERR_OPEN        11 /* errno is set */
#define SPNOTES_ERR_DELETE      12 /* errno is set */

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */

/* = spnotes_t = */

/*
 * Initialize the given 'spnotes_t' struct pointer instance with the given root
 * location.
 *
 * Returns 0 on error and sets the `spnotes_err` with the error.
 * The error can be:
 * 'SPNOTES_ERR_NULL_PTR' - NULL is passed on `root_location`.
 */
SPNOTES_DEF int
spnotes_init(spnotes_t *instance, const char *root_location);

/*
 * Destructor for the 'spnotes_t' instance.
 *
 * Completely safe to pass a NULL pointer.
 */
SPNOTES_DEF void
spnotes_free(spnotes_t *instance);

/* = Category = */

/*
 * Fills up the 'categs' and 'categs_c' in the given `instance` with all the
 * categories found.
 *
 * See the comments on `spnotes_categs_fill_filter()` to know about the return
 * values.
 */
SPNOTES_DEF int
spnotes_categs_fill(spnotes_t *instance);

/*
 * Fills up the 'categs' and 'categs_c' in the given `instance`. BUT only those
 * category having title whose return value when passed to the function
 * `filter_func` is positive gets added.
 *
 * Passing NULL to `filter` or `filter_func` is equivalent to calling
 * 'spnotes_categs_fill()'.
 *
 * Returns the number of categories found OR -1 on error and sets the
 * `spnotes_err` with the error.
 * The error can be:
 * 'SPNOTES_ERR_INVALID_LOC' - Invalid location to the root notes.
 * 'SPNOTES_ERR_MALLOC' - Couldn't allocate required memory.
 * 'SPNOTES_ERR_REALLOC' - Couldn't reallocate required memory.
 * 'SPNOTES_ERR_FILE_STAT' - Couldn't get the required info of file.
 * 'SPNOTES_ERR_DIR_READ' - Couldn't read the files on directory.
 */
SPNOTES_DEF int
spnotes_categs_fill_filter(spnotes_t *instance, char *filter,
                           int (*filter_func)(const char *, const char *));

/*
 * Compare function to sort the categories found in descending order of last
 * modified.
 *
 * To be passed to a 'qsort()' function.
 */
SPNOTES_DEF int
spnotes_categs_compare_last_modified(const void *categ1, const void *categ2);

/*
 * Compare function to sort the categories found in ascending alphabetical
 * order.
 *
 * To be passed to a 'qsort()' function.
 */
SPNOTES_DEF int
spnotes_categs_compare_alphabetically(const void *categ1, const void *categ2);

/*
 * Sorts the categories found in descending order of last modified.
 *
 * Completely safe to pass a NULL pointer or a spnotes instance whose categories
 * hasn't been filled yet.
 */
SPNOTES_DEF void
spnotes_categs_sort_last_modified(spnotes_t *instance);

/*
 * Sorts the categories found in ascending alphabetical order.
 *
 * Completely safe to pass a NULL pointer or a spnotes instance whose categories
 * hasn't been filled yet.
 */
SPNOTES_DEF void
spnotes_categs_sort_alphabetically(spnotes_t *instance);

/*
 * Search for a given category in the note system.
 *
 * On success, returns pointer to the category instance else returns NULL if
 * the categories array isn't filled yet.
 */
SPNOTES_DEF spnotes_categ *
spnotes_categs_search(spnotes_t instance, const char *title);

/*
 * Creates a new category in the note system.
 *
 * Fills up `new_loc` with the path on disk where the category was added. NULL
 * can be passed to ignore it.
 *
 * Returns 0 on error and sets the `spnotes_err` with the error.
 * The error can be:
 * 'SPNOTES_ERR_REDECLARE' - A category with the given title already exists.
 * 'SPNOTES_ERR_MKDIR' - Couldn't create a directory in the required location.
 */
SPNOTES_DEF int
spnotes_categs_add(spnotes_t instance, const char *title, char *new_loc);

/*
 * Delete the given category in the note system.
 *
 * Returns 0 on error and sets the `spnotes_err` with the error.
 * The error can be:
 * 'SPNOTES_ERR_DELETE' - The category directory couldn't be deleted.
 */
SPNOTES_DEF int
spnotes_categs_remove(spnotes_categ categ);

/* = Note = */

/*
 * Fills up the 'title', 'description' and 'has_description' in the given
 * `spnotes_note` with the information in yaml-headers of the given md file.
 *
 * Returns -1 on error (setting the 'spnotes_err'), 0 if none/file cannot be
 * read, 1 if only title, or 2 if both title and description is found.
 */
SPNOTES_DEF int
spnotes_note_fill_title_desc(spnotes_note *note, char *md_loc);

/*
 * Fills up the 'notes' and 'notes_c' in the given `spnotes_categ` with all the
 * notes found.
 *
 * See the comments on `spnotes_notes_fill_filter()` to know about the return
 * values.
 */
SPNOTES_DEF int
spnotes_notes_fill(spnotes_categ *categ);

/*
 * Fills up the 'notes' and 'notes_c' in the given `spnotes_categ` with all the
 * notes found. BUT only those notes having title whose return value when passed
 * to the function `filter_func` is positive gets added.
 *
 * Passing NULL to `filter` or `filter_func` is equivalent to calling
 * 'spnotes_categs_fill()'.
 *
 * Returns the number of notes found OR -1 on error and sets the `spnotes_err`
 * with the error.
 * The error can be:
 * 'SPNOTES_ERR_INVALID_LOC' - Invalid location to the category.
 * 'SPNOTES_ERR_MALLOC' - Couldn't allocate required memory.
 * 'SPNOTES_ERR_REALLOC' - Couldn't reallocate required memory.
 * 'SPNOTES_ERR_FILE_STAT' - Couldn't get the required info of file.
 * 'SPNOTES_ERR_DIR_READ' - Couldn't read the files on directory.
 */
SPNOTES_DEF int
spnotes_notes_fill_filter(spnotes_categ *categ, char *filter,
                          int (*filter_func)(const char *, const char *));

/*
 * Compare function to sort the notes found in descending order of last
 * modified.
 *
 * To be passed to a 'qsort()' function.
 */
SPNOTES_DEF int
spnotes_notes_compare_last_modified(const void *note1, const void *note2);

/*
 * Compare function to sort the notes found in ascending alphabetical order.
 *
 * To be passed to a 'qsort()' function.
 */
SPNOTES_DEF int
spnotes_notes_compare_alphabetically(const void *note1, const void *note2);

/*
 * Sorts the notes found in descending order of last modified.
 *
 * Completely safe to pass a NULL pointer or a spnotes category whose notes
 * hasn't been filled yet.
 */
SPNOTES_DEF void
spnotes_notes_sort_last_modified(spnotes_categ *categ);

/*
 * Sorts the notes found in ascending alphabetical order.
 *
 * Completely safe to pass a NULL pointer or a spnotes category whose notes
 * hasn't been filled yet.
 */
SPNOTES_DEF void
spnotes_notes_sort_alphabetically(spnotes_categ *categ);

/*
 * Search for a note of given title in the given category in the note system.
 *
 * On success, returns pointer to the note instance else returns NULL if the
 * notes array isn't filled yet.
 */
SPNOTES_DEF spnotes_note *
spnotes_notes_search(spnotes_categ categ, const char *title);

/*
 * Creates a new note in the note system.
 *
 * The new file created is a empty file. Implementation to add content to the
 * file are expected to be added. See the layout section of spnotes for more
 * info on which parts are compulsory to be added.
 *
 * Fills up `new_loc` with the path on disk where the note was added. NULL can
 * be passed to ignore it. But idk why would you want to ever ignore it.
 * Without the proper layout on the file, the note won't be shown on the
 * system.
 *
 * Returns 0 on error and sets the `spnotes_err` with the error.
 * The error can be:
 * 'SPNOTES_ERR_OPEN' - Couldn't create a file in the required location.
 */
SPNOTES_DEF int
spnotes_notes_add(spnotes_categ categ, char *new_loc);

/*
 * Delete the given note in the note system.
 *
 * Returns 0 on error and sets the `spnotes_err` with the error.
 * The error can be:
 * 'SPNOTES_ERR_DELETE' - The note file couldn't be deleted.
 */
SPNOTES_DEF int
spnotes_notes_remove(spnotes_note note);

/* = Errors = */

/* Returns the string representation of the error in 'splnotes_err'. */
SPNOTES_DEF char *
spnotes_errorstr(void);

#endif /* SPNOTES_H */

/*
 ===============================================================================
 |                             IMPLEMENTATION MODE                             |
 ===============================================================================
 */

#ifdef SPNOTES_IMPL

/*
 ===============================================================================
 |                          Function Implementations                           |
 ===============================================================================
 */

/* = spnotes_t = */

SPNOTES_DEF int
spnotes_init(spnotes_t *instance, const char *root_location)
{
	if (root_location == NULL) {
		spnotes_err = SPNOTES_ERR_NULL_PTR;
		return 0;
	}

	instance->root_location = strdup(root_location);
	if (instance->root_location[strlen(instance->root_location) - 1] != '/')
		strcat(instance->root_location, "/");
	instance->categs = NULL;
	instance->categs_c = 0;

	spnotes_err = SPNOTES_ERR_NONE;

	return 1;
}

SPNOTES_DEF void
spnotes_free(spnotes_t *instance)
{
	if (instance == NULL)
		return;

	for (size_t i = 0; i < instance->categs_c; i++) {
		for (size_t j = 0; j < instance->categs[i].notes_c; j++)
			if (instance->categs[i].notes[j].has_description)
				free(instance->categs[i].notes[j].description);
		free(instance->categs[i].notes);
	}
	free(instance->categs);

	free(instance->root_location);
}

/* = Category = */

SPNOTES_DEF int
spnotes_categs_fill(spnotes_t *instance)
{
	return spnotes_categs_fill_filter(instance, NULL, NULL);
}

SPNOTES_DEF int
spnotes_categs_fill_filter(spnotes_t *instance, char *filter,
                           int (*filter_func)(const char *, const char *))
{
	DIR *dir = opendir(instance->root_location);
	if (dir == NULL) {
		spnotes_err = SPNOTES_ERR_INVALID_LOC;
		return -1;
	}

	int            categs_c = 0, mcategs_c = 128;
	spnotes_categ *categs = malloc(mcategs_c * sizeof(spnotes_categ));
	if (categs == NULL) {
		spnotes_err = SPNOTES_ERR_MALLOC;
		return -1;
	}

	/* start reading the directory */
	errno = 0;
	struct dirent *dirent;
	while ((dirent = readdir(dir))) {
		/* filter out files starting with "." */
		if (dirent->d_name[0] == '.')
			continue;
		/* filter out non-dir */
		if (dirent->d_type != DT_DIR)
			continue;
		/* filter with the `filter_func()` (if eligible) */
		if (filter != NULL && filter_func != NULL &&
		    filter_func(dirent->d_name, filter) <= 0)
			continue;

		/* check if the size of dynamic array has to be increased */
		if (categs_c == mcategs_c - 1) {
			spnotes_categ *temp_categs = realloc(
				categs, mcategs_c * 2 * sizeof(spnotes_categ));
			if (temp_categs == NULL) {
				instance->categs   = categs;
				instance->categs_c = categs_c;

				spnotes_err = SPNOTES_ERR_REALLOC;
				closedir(dir);
				return -1;
			}
			categs = temp_categs;
			mcategs_c *= 2;
		}
		strcpy(categs[categs_c].title, dirent->d_name);
		categs[categs_c].notes            = NULL;
		categs[categs_c].notes_c          = 0;
		categs[categs_c].spnotes_instance = instance;

		char path[PATH_MAX];
		snprintf(path, PATH_MAX, "%s%s/", instance->root_location,
		         dirent->d_name);

		strcpy(categs[categs_c].path, path);

		/* get the last modified date */
		struct stat categ_stat;
		if (stat(path, &categ_stat) != 0) {
			instance->categs   = categs;
			instance->categs_c = categs_c;

			spnotes_err = SPNOTES_ERR_FILE_STAT;
			closedir(dir);
			return -1;
		}
		categs[categs_c].last_modified = categ_stat.st_mtim;

		categs_c++;
	}
	if (errno != 0) {
		instance->categs   = categs;
		instance->categs_c = categs_c;

		spnotes_err = SPNOTES_ERR_DIR_READ;
		closedir(dir);
		return -1;
	}

	closedir(dir);

	instance->categs   = categs;
	instance->categs_c = categs_c;
	return categs_c;
}

SPNOTES_DEF int
spnotes_categs_compare_last_modified(const void *categ1, const void *categ2)
{
	struct timespec *tm1_ts = &(((spnotes_categ *)categ1)->last_modified);
	struct timespec *tm2_ts = &(((spnotes_categ *)categ2)->last_modified);

	if (tm1_ts->tv_sec == tm2_ts->tv_sec) {
		if (tm1_ts->tv_nsec > tm2_ts->tv_nsec)
			return -1;
		else
			return 1;
	}

	if (tm1_ts->tv_sec > tm2_ts->tv_sec)
		return -1;
	return 1;
}

SPNOTES_DEF int
spnotes_categs_compare_alphabetically(const void *categ1, const void *categ2)
{
	char *categ1_title = ((spnotes_categ *)categ1)->title;
	char *categ2_title = ((spnotes_categ *)categ2)->title;

	return strcmp(categ1_title, categ2_title);
}

SPNOTES_DEF void
spnotes_categs_sort_last_modified(spnotes_t *instance)
{
	if (instance == NULL || instance->categs == NULL)
		return;

	qsort(instance->categs, instance->categs_c, sizeof(spnotes_categ),
	      spnotes_categs_compare_last_modified);
}

SPNOTES_DEF void
spnotes_categs_sort_alphabetically(spnotes_t *instance)
{
	if (instance == NULL || instance->categs == NULL)
		return;

	qsort(instance->categs, instance->categs_c, sizeof(spnotes_categ),
	      spnotes_categs_compare_alphabetically);
}

SPNOTES_DEF spnotes_categ *
spnotes_categs_search(spnotes_t instance, const char *title)
{
	if (!(instance.categs)) {
		spnotes_err = SPNOTES_ERR_NOT_FILLED;
		return NULL;
	}

	for (size_t i = 0; i < instance.categs_c; i++) {
		if (!strcmp(instance.categs[i].title, title))
			return (instance.categs + i);
	}
	return NULL;
}

SPNOTES_DEF int
spnotes_categs_add(spnotes_t instance, const char *title, char *new_loc)
{
	if (spnotes_categs_search(instance, title)) {
		spnotes_err = SPNOTES_ERR_REDECLARE;
		return 0;
	}

	char path[PATH_MAX];
	snprintf(path, PATH_MAX, "%s%s", instance.root_location, title);

	if (mkdir(path, 0777) != 0) {
		spnotes_err = SPNOTES_ERR_MKDIR;
		return 0;
	}

	if (new_loc)
		strcpy(new_loc, path);

	return 1;
}

/* directory unlink function (for 'rmrf()') */
int
unlink_cb(const char *fpath, const struct stat *sb, int typeflag,
          struct FTW *ftwbuf)
{
	(void)sb;
	(void)typeflag;
	(void)ftwbuf;

	return remove(fpath);
}

/* recursive directory deletion (for 'spnotes_categs_remove()') */
int
rmrf(const char *path)
{
	return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

SPNOTES_DEF int
spnotes_categs_remove(spnotes_categ categ)
{
	if (rmrf(categ.path) == -1) {
		spnotes_err = SPNOTES_ERR_DELETE;
		return 0;
	}
	return 1;
}

/* = Note = */

SPNOTES_DEF int
spnotes_note_fill_title_desc(spnotes_note *note, char *md_loc)
{
	int ret = 0;

	FILE *fp = fopen(md_loc, "r");
	if (fp == NULL)
		return ret;

	errno = 0;

	/* main logic */
	char buffer[4098], *tmp_ptr;
	fgets(buffer, sizeof(buffer), fp);

	/* continue only if the first line is a starting yaml header */
	if (strcmp(buffer, "---\n")) {
		fclose(fp);
		return ret;
	}

	while ((fgets(buffer, sizeof(buffer), fp)) != NULL) {
		/* stop when we encounter the ending yaml header */
		if (!strcmp(buffer, "---\n"))
			break;

		/* parse title */
		if ((strstr(buffer, "title:")) == buffer) { /* if ^title: */
			tmp_ptr = strchr(buffer, ':') + 1;

			while (*tmp_ptr == ' ') /* point to the 1st nonspace */
				tmp_ptr++;

			if (strcmp(tmp_ptr, "\n")) {
				strncpy(note->title, tmp_ptr,
				        strchr(tmp_ptr, '\n') - tmp_ptr);
				ret = 1;
			}
			continue;
		}

		/* parse description (only if title was found before) */
		if (ret != 1)
			continue;

		if ((strstr(buffer, "description:")) == buffer) {
			tmp_ptr = strchr(buffer, ':') + 1;

			while (*tmp_ptr == ' ') /* point to the 1st nonspace */
				tmp_ptr++;

			if (strcmp(tmp_ptr, "\n")) {
				note->description =
					strndup(tmp_ptr, strchr(tmp_ptr, '\n') -
				                                 tmp_ptr);
				ret = 2;
			}
		}
	}
	if (errno != 0) { /* fgets return NULL on error too */
		spnotes_err = SPNOTES_ERR_FILE_READ;
		fclose(fp);
		return -1;
	}

	note->has_description = (ret == 2);

	fclose(fp);
	return ret;
}

SPNOTES_DEF int
spnotes_notes_fill(spnotes_categ *categ)
{
	return spnotes_notes_fill_filter(categ, NULL, NULL);
}

SPNOTES_DEF int
spnotes_notes_fill_filter(spnotes_categ *categ, char *filter,
                          int (*filter_func)(const char *, const char *))
{
	DIR *dir = opendir(categ->path);
	if (dir == NULL) {
		spnotes_err = SPNOTES_ERR_INVALID_LOC;
		return -1;
	}

	int           notes_c = 0, mnotes_c = 128;
	spnotes_note *notes = malloc(mnotes_c * sizeof(spnotes_note));
	if (notes == NULL) {
		spnotes_err = SPNOTES_ERR_MALLOC;
		return -1;
	}

	/* start reading the directory */
	errno = 0;
	struct dirent *dirent;
	while ((dirent = readdir(dir))) {
		/* filter out files starting with "." */
		if (dirent->d_name[0] == '.')
			continue;
		/* filter out dir */
		if (dirent->d_type == DT_DIR)
			continue;
		/* filter out non-md files */
		if (!strstr(dirent->d_name, ".md") &&
		    !strstr(dirent->d_name, ".MD"))
			continue;

		/* check if the size of dynamic array has to be increased */
		if (notes_c == mnotes_c - 1) {
			spnotes_note *temp_notes = realloc(
				notes, mnotes_c * 2 * sizeof(spnotes_note));
			if (temp_notes == NULL) {
				categ->notes   = notes;
				categ->notes_c = notes_c;

				spnotes_err = SPNOTES_ERR_REALLOC;
				closedir(dir);
				return -1;
			}
			notes = temp_notes;
			mnotes_c *= 2;
		}

		/* fill title and description */
		char path[PATH_MAX];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wformat-truncation"
		snprintf(path, PATH_MAX, "%s%s", categ->path,
		         dirent->d_name);
#pragma GCC diagnostic pop
		/* filter out md files not having title */
		if (spnotes_note_fill_title_desc(&notes[notes_c], path) <
		    1)
			continue;

		/* filter with the `filter_func()` (if eligible) */
		if (filter != NULL && filter_func != NULL &&
		    filter_func(notes[notes_c].title, filter) <= 0) {
			free(notes[notes_c].description);
			continue;
		}

		strcpy(notes[notes_c].path, path);
		notes[notes_c].categ = categ;

		/* get the last modified date */
		struct stat note_stat;
		if (stat(path, &note_stat) != 0) {
			categ->notes   = notes;
			categ->notes_c = notes_c;

			spnotes_err = SPNOTES_ERR_FILE_STAT;
			closedir(dir);
			return -1;
		}
		notes[notes_c].last_modified = note_stat.st_mtim;

		notes_c++;
	}
	if (errno != 0) {
		spnotes_err = SPNOTES_ERR_DIR_READ;
		closedir(dir);
		return -1;
	}

	closedir(dir);

	categ->notes   = notes;
	categ->notes_c = notes_c;
	return notes_c;
}

SPNOTES_DEF int
spnotes_notes_compare_last_modified(const void *note1, const void *note2)
{
	struct timespec *tm1_ts = &(((spnotes_note *)note1)->last_modified);
	struct timespec *tm2_ts = &(((spnotes_note *)note2)->last_modified);

	if (tm1_ts->tv_sec == tm2_ts->tv_sec) {
		if (tm1_ts->tv_nsec > tm2_ts->tv_nsec)
			return -1;
		else
			return 1;
	}

	if (tm1_ts->tv_sec > tm2_ts->tv_sec)
		return -1;
	return 1;
}

SPNOTES_DEF int
spnotes_notes_compare_alphabetically(const void *note1, const void *note2)
{
	char *note1_title = ((spnotes_note *)note1)->title;
	char *note2_title = ((spnotes_note *)note2)->title;

	return strcmp(note1_title, note2_title);
}

SPNOTES_DEF void
spnotes_notes_sort_last_modified(spnotes_categ *categ)
{
	if (categ == NULL || categ->notes == NULL)
		return;

	qsort(categ->notes, categ->notes_c, sizeof(spnotes_note),
	      spnotes_notes_compare_last_modified);
}

SPNOTES_DEF void
spnotes_notes_sort_alphabetically(spnotes_categ *categ)
{
	if (categ == NULL || categ->notes == NULL)
		return;

	qsort(categ->notes, categ->notes_c, sizeof(spnotes_note),
	      spnotes_notes_compare_alphabetically);
}

SPNOTES_DEF spnotes_note *
spnotes_notes_search(spnotes_categ categ, const char *title)
{
	if (!(categ.notes)) {
		spnotes_err = SPNOTES_ERR_NOT_FILLED;
		return NULL;
	}

	for (size_t i = 0; i < categ.notes_c; i++) {
		if (!strcmp(categ.notes[i].title, title))
			return (categ.notes + i);
	}
	return NULL;
}

SPNOTES_DEF int
spnotes_notes_add(spnotes_categ categ, char *new_loc)
{
	char path[PATH_MAX];
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wformat-truncation"
	snprintf(path, PATH_MAX, "%s%ld.md", categ.path,
	         (unsigned long)time(NULL));
#pragma GCC diagnostic pop

	int fd = open(path, O_WRONLY | O_CREAT, DEFFILEMODE);
	if (fd == -1) {
		spnotes_err = SPNOTES_ERR_OPEN;
		return 0;
	}
	if (close(fd) == -1) {
		spnotes_err = SPNOTES_ERR_OPEN;
		return 0;
	}

	if (new_loc)
		strcpy(new_loc, path);

	return 1;
}

SPNOTES_DEF int
spnotes_notes_remove(spnotes_note note)
{
	if (remove(note.path) == -1) {
		spnotes_err = SPNOTES_ERR_DELETE;
		return 0;
	}
	return 1;
}

/* = Errors = */

SPNOTES_DEF char *
spnotes_errorstr(void)
{
	switch (spnotes_err) {
	case SPNOTES_ERR_NONE:
		return "No error";
	case SPNOTES_ERR_NULL_PTR:
		return "NULL pointer passed";
	case SPNOTES_ERR_MALLOC:
	case SPNOTES_ERR_REALLOC:
		return "Cannot allocate required memory";
	case SPNOTES_ERR_DIR_READ:
		return "Cannot read the directory";
	case SPNOTES_ERR_INVALID_LOC:
		return "Invalid location";
	case SPNOTES_ERR_FILE_READ:
		return "Cannot read the file";
	case SPNOTES_ERR_FILE_STAT:
		return "Cannot read the file stat";
	case SPNOTES_ERR_REDECLARE:
		return "Given name was already declared";
	case SPNOTES_ERR_NOT_FILLED:
		return "Required field wasn't filled";
	case SPNOTES_ERR_MKDIR:
		return "Cannot create the directory";
	case SPNOTES_ERR_OPEN:
		return "Cannot open the file";
	case SPNOTES_ERR_DELETE:
		return "Cannot delete the file";
	}

	return "No error";
}

#endif /* SPNOTES_IMPL */

/*
 ===============================================================================
 |                 License - Public Domain (www.unlicense.org)                 |
 ===============================================================================
 *
 * This is free and unencumbered software released into the public domain.
 *
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 *
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * For more information, please refer to <http://unlicense.org/>
 */
