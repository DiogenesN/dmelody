#include <glib.h>
#include <stdio.h>
#include <string.h>
#include "externvars.h"
#include "protofunctions.h"
#include "getvaluefromconf.h"

/// open directory action
static void opendir_cb() {
	openDirState = 1;

	GtkStringObject *myItem;
	myItem = gtk_single_selection_get_selected_item(singleSelectionDirs);
	const gchar *selectedItem = gtk_string_object_get_string(myItem);
	const gchar *musicPath = get_char_value_from_conf(pathToConfigDmelody, "musicpath");

	if (strlen(fullPathToSelDir) == 0) {
		strcpy((char *)fullPathToSelDir, musicPath);
		strcat((char *)fullPathToSelDir, "/");
		strcat((char *)fullPathToSelDir, selectedItem);
	}
	else if (strlen(fullPathToSelDir) != 0 && addToPlaylistState == 0) {
		strcat((char *)fullPathToSelDir, "/");
		strcat((char *)fullPathToSelDir, selectedItem);
	}
	else if (strlen(fullPathToSelDir) != 0 && addToPlaylistState == 1) {
		//g_print("Nothing to do\n");
		strcpy((char *)fullPathToSelDir, musicPath);
		strcat((char *)fullPathToSelDir, "/");
		strcat((char *)fullPathToSelDir, selectedItem);
	}

	g_print("Opening: %s\n", fullPathToSelDir);
	populate_dirs(fullPathToSelDir);
	gtk_string_list_splice(directories,
							0,
							g_list_model_get_n_items(dirsListModel),
							(const char * const*)dirs);

	// Free allocated memory
	for (size_t i = 0; dirs[i] != NULL; i++) {
		free(dirs[i]);
	}
	free(dirs);
	free((void *)musicPath);
	addToPlaylistState = 0;
}
