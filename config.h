/* Defalt notes location - Will get overridden if  */
static char *notes_root_loc = "/home/safal/docs/notes/";

/* Template to add when new note is created (simple printf macro) */
#define NEW_NOTE_TEMPLATE \
	"---\ntitle: %s\ndescription: %s\n---", option_note, option_desc
#define NEW_NOTE_TEMPLATE_TITLE_ONLY \
	"---\ntitle: %s\n---", option_note
