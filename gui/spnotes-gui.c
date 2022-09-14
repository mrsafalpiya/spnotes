#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>

#include <iup.h>

#define SPNOTES_IMPL
#include "../spnotes.h"

/*
 ===============================================================================
 |                              Global Variables                               |
 ===============================================================================
 */

/* = SPNOTES = */

static spnotes_t spn_instance;
static char     *notes_root_loc = "/home/safal/docs/notes/";

static spnotes_categ *categ_sel = NULL;
static spnotes_note  *note_sel  = NULL;

/* = ELEMENTS = */

static Ihandle *elem_multitext = NULL;

static Ihandle *elem_flatlist_categ = NULL;
static Ihandle *elem_flatlist_note  = NULL;

/*
 ===============================================================================
 |                            Function Declarations                            |
 ===============================================================================
 */

/* = CALLBACKS = */

int
cb_exit(Ihandle *self);

int
cb_font(Ihandle *self);

int
cb_about(Ihandle *self);

int
cb_list_categ_changed(Ihandle *self, char *text, int item, int state);

int
cb_list_note_changed(Ihandle *self, char *text, int item, int state);

/* = FILE HANDLING = */

char *
file_read(const char *filename);

/*
 ===============================================================================
 |                          Function Implementations                           |
 ===============================================================================
 */

/* = CALLBACKS = */

int
cb_exit(Ihandle *self)
{
	(void)self;

	return IUP_CLOSE;
}

int
cb_font(Ihandle *self)
{
	(void)self;

	Ihandle *dlg_font = IupFontDlg();

	char *font = IupGetAttribute(elem_multitext, "FONT");
	IupSetStrAttribute(dlg_font, "VALUE", font);
	IupPopup(dlg_font, IUP_CENTER, IUP_CENTER);

	if (IupGetInt(dlg_font, "STATUS") == 1) {
		char *font = IupGetAttribute(dlg_font, "VALUE");
		IupSetStrAttribute(elem_multitext, "FONT", font);
	}

	IupDestroy(dlg_font);
	return IUP_DEFAULT;
}

int
cb_about(Ihandle *self)
{
	(void)self;

	IupMessage("About",
	           "spnotes-gui\n\nAuthor:\nSafal Piya (@mrsafalpiya)");

	return IUP_DEFAULT;
}

int
cb_list_categ_changed(Ihandle *self, char *text, int item, int state)
{
	(void)self;
	(void)text;
	(void)state;

	/* empty the list first */
	IupSetAttributeId(elem_flatlist_note, "", 1, NULL);

	/* fill the list */
	categ_sel = &(spn_instance.categs[item - 1]);
	if (categ_sel->notes == NULL)
		spnotes_notes_fill(categ_sel);
	for (size_t i = 0; i < categ_sel->notes_c; i++) {
		IupSetAttributeId(elem_flatlist_note, "", i + 1,
		                  categ_sel->notes[i].title);
	}

	return IUP_DEFAULT;
}

int
cb_list_note_changed(Ihandle *self, char *text, int item, int state)
{
	(void)self;
	(void)text;
	(void)state;

	if (categ_sel == NULL)
		return IUP_DEFAULT;

	note_sel = &(categ_sel->notes[item - 1]);

	char *str = file_read(note_sel->path);
	if (str) {
		IupSetStrAttribute(elem_multitext, "VALUE", str);
		free(str);
	}

	return IUP_DEFAULT;
}

/* = FILE HANDLING = */

char *
file_read(const char *filename)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		IupMessagef("Error", "Can't open file: %s", filename);
		return NULL;
	}

	/* calculate file size */
	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/* allocate memory for the file contents + NULL terminator */
	char *str = malloc(size + 1);
	/* read all data at once */
	fread(str, size, 1, fp);
	/* set the NULL terminator */
	str[size] = 0;

	if (ferror(fp))
		IupMessagef("Error", "Couldn't read the file: %s", filename);

	fclose(fp);
	return str;
}

int
main(int argc, char **argv)
{
	IupOpen(&argc, &argv);

	/* = MENU = */

	Ihandle *item_exit  = IupItem("Exit", NULL);
	Ihandle *item_font  = IupItem("Font", NULL);
	Ihandle *item_about = IupItem("About", NULL);

	IupSetCallback(item_exit, "ACTION", (Icallback)cb_exit);
	IupSetCallback(item_font, "ACTION", (Icallback)cb_font);
	IupSetCallback(item_about, "ACTION", (Icallback)cb_about);

	Ihandle *menu_file   = IupMenu(item_exit, NULL);
	Ihandle *menu_format = IupMenu(item_font, NULL);
	Ihandle *menu_help   = IupMenu(item_about, NULL);

	Ihandle *submenu_file   = IupSubmenu("File", menu_file);
	Ihandle *submenu_format = IupSubmenu("Format", menu_format);
	Ihandle *submenu_help   = IupSubmenu("Help", menu_help);

	Ihandle *menu =
		IupMenu(submenu_file, submenu_format, submenu_help, NULL);

	/* = ELEMENTS = */

	/* category list */
	elem_flatlist_categ = IupFlatList();
	IupSetStrAttribute(elem_flatlist_categ, "SIZE", "100x50");
	IupSetCallback(elem_flatlist_categ, "FLAT_ACTION",
	               cb_list_categ_changed);

	/* fill category list */
	spnotes_init(&spn_instance, notes_root_loc);
	spnotes_categs_fill(&spn_instance);

	for (size_t i = 0; i < spn_instance.categs_c; i++) {
		IupSetAttributeId(elem_flatlist_categ, "", i + 1,
		                  spn_instance.categs[i].title);
	}

	/* note list */
	elem_flatlist_note = IupFlatList();
	IupSetStrAttribute(elem_flatlist_note, "SIZE", "100x50");
	IupSetStrAttribute(elem_flatlist_note, "EXPAND", "HORIZONTAL");
	IupSetAttributeId(elem_flatlist_note, "", 1, "Select a category");
	IupSetCallback(elem_flatlist_note, "FLAT_ACTION", cb_list_note_changed);

	/* multitext */
	elem_multitext = IupText(NULL);
	IupSetAttribute(elem_multitext, "MULTILINE", "YES");
	IupSetAttribute(elem_multitext, "EXPAND", "YES");
	IupSetAttribute(elem_multitext, "READONLY", "YES");

	/* layout */
	Ihandle *hbox = IupHbox(elem_flatlist_categ, elem_flatlist_note, NULL);
	Ihandle *vbox = IupVbox(hbox, elem_multitext, NULL);

	Ihandle *dlg_main = IupDialog(vbox);
	IupSetAttributeHandle(dlg_main, "MENU", menu);
	IupSetAttribute(dlg_main, "TITLE", "spnotes-gui");
	IupSetAttribute(dlg_main, "SIZE", "500x250");

	/* = IUP = */

	IupShowXY(dlg_main, IUP_CENTER, IUP_CENTER);

	IupMainLoop();

	/* = EXIT = */

	spnotes_free(&spn_instance);
	IupClose();
	return EXIT_SUCCESS;
}
