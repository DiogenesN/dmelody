/* DMelody
 * Depends: mpv, gawk, perl, libimage-exiftool-perl
 * functionality:
 	cli: dmelody --help, dmelody --previous, dmelody --toggle-pause, dmelody --next, dmelody --quit
 	1) file browser
 	2) cover art
 	3) AutoPlay on start Switch
 	4) Automatically save window size
 	5) Repeat current track switch
 	6) Automatically show control bar switch
 	7) Show/Hide control bar (prev, play/pause, next, hide control, toggle view, quit)
 	8) slider showing remaining time/total time
 	9) Playlist1, Playlist2, Playlist3, the label of the playlists change to folder name
 	10) playlist viewer: No | Artist | Title | Album | duration
 	11) Buttons: Close | Settings | Show conteol bar | Previous | Play/Pause | Next | Repeat
 */

#include <glib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gio/gio.h>

#include "configsgen.c"
#include "externvars.h"
#include "sendcmdtompv.h"
#include "protofunctions.h"
#include "replacenthline.h"
#include "getvaluefromconf.h"
#include "dmelody_settings.c"
#include "getnumberoflines.h"
#include "populateallstrings.h"

#include "x1_opendir_cb.c"
#include "y1_addtopl1_cb.c"
#include "y2_addtopl2_cb.c"
#include "y3_addtopl3_cb.c"
#include "open_control_bar.c"
#include "z1_replacepl1_cb.c"
#include "z2_replacepl2_cb.c"
#include "z3_replacepl3_cb.c"
#include "on_track_activate.c"
#include "on_playlist1_l_click.c"
#include "on_playlist2_l_click.c"
#include "on_playlist3_l_click.c"

GtkExpression *expressionWidth;
GtkExpressionWatch *watchWidth;

GtkExpression *expressionHeight;
GtkExpressionWatch *watchHeight;

GtkExpression *expressionMaximized;
GtkExpressionWatch *watchMaximized;

GtkExpressionWatch *watchRepeatSwitch;

/// this function is used to compare and alphabetic sort the directories in Music dir
int compare_names(const void *a, const void *b) {
	const char **entry_a = (const char **)a;
	const char **entry_b = (const char **)b;
	return strcmp(*entry_a, *entry_b);
}

/// this function pupulates dirs variable with name of subdirectories in Music directory
void populate_dirs(const gchar *musicPath) {
	size_t num_entries = 0;
	size_t max_entries = 10;

	struct dirent *entry;
	dirs = malloc(max_entries * sizeof(char *));

	DIR *dir = opendir(musicPath);
	if (dir == NULL) {
		perror("opendir");
		dirs[0] = g_strdup("Wrong directory name");
		dirs[1] = NULL;
		//free((void *)musicPath);

		closedir(dir);
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
											strcmp(entry->d_name, "..") != 0) {
			// Check if it's a directory and not "." or ".."
			dirs[num_entries] = strdup(entry->d_name);
			num_entries++;

			// Resize the array if needed
			if (num_entries == max_entries) {
				max_entries *= 2;
				dirs = realloc(dirs, max_entries * sizeof(char *));
    		}
		}
	}
	// extremely important to NULL terminale the chararray
	dirs[num_entries] = NULL;

	// Sort the array based on names
	qsort(dirs, num_entries, sizeof(char *), compare_names);

	//free((void *)musicPath);
	closedir(dir);
}

/// do the actual resize on maximize
gboolean get_currwnt_height() {
	gtk_widget_set_size_request(browserScrolledWindow, 0,
														gtk_widget_get_height(dmelodyWindow) - 460);
	gtk_widget_set_size_request(trackListScrolledWindow, 0,
														gtk_widget_get_height(dmelodyWindow) - 140);
	return G_SOURCE_REMOVE | FALSE;
}

/// resize on maximize
static void notif_max() {
	g_timeout_add_seconds(1, get_currwnt_height, NULL);
}

/// repeat switch action
static void notifyRepeatSw(gpointer data) {
	gchar *writeRepeatToConf = g_strdup_printf("repeat=%d", gtk_switch_get_state(GTK_SWITCH(data)));
	replacenthline(pathToConfigDmelody, 4, writeRepeatToConf);
	g_free(writeRepeatToConf);

	gint nrOfLines;
	tabClicked = get_int_value_from_conf(pathToConfigDmelody, "lastTabClicked");

	if (tabClicked == 1) {
		nrOfLines = get_number_of_lines(pathToPlaylist1);
	}
	else if (tabClicked == 2) {
		nrOfLines = get_number_of_lines(pathToPlaylist2);
	}
	else {
		nrOfLines = get_number_of_lines(pathToPlaylist3);
	}

	// repeat command
	if (gtk_widget_is_sensitive(repeatTrack) && isPlaying && nrOfLines >= 1) {
		if (get_int_value_from_conf(pathToConfigDmelody, "repeat") == 1) {
			// starts infinite track loop
			g_print("Repeat is on\n");
			send_mpv_cmd("/tmp/mpvsocket", "cycle-values loop-file \"inf\" \"yes\"\n");
		}
		else {
			// stops loop
			send_mpv_cmd("/tmp/mpvsocket", "cycle-values loop-file \"inf\" \"no\"\n");
		}
	}
}

/// on window resize signal handler
static void notify(gpointer data) {
	(void)data;

	// width
  	GValue valueWidth = G_VALUE_INIT;
	gtk_expression_watch_evaluate(watchWidth, &valueWidth);

	// height
  	GValue valueHeight = G_VALUE_INIT;
	gtk_expression_watch_evaluate(watchHeight, &valueHeight);

	if (G_VALUE_HOLDS_INT(&valueWidth) && G_VALUE_HOLDS_INT(&valueHeight)) {
		gint widthValue  = g_value_get_int(&valueWidth);
		gint heightValue = g_value_get_int(&valueHeight);

		gtk_widget_set_size_request(browserScrolledWindow, 0, heightValue - 460);
		gtk_widget_set_size_request(trackListScrolledWindow, 0, heightValue - 140);

		gchar windowWidth[100];
		gchar windowHeight[100];
		snprintf(windowWidth, sizeof(windowWidth), "width=%d", widthValue);
		snprintf(windowHeight, sizeof(windowHeight), "height=%d", heightValue);
		replacenthline(pathToConfigDmelody, 1, windowWidth);
		replacenthline(pathToConfigDmelody, 2, windowHeight);

		g_value_unset(&valueWidth);
		g_value_unset(&valueHeight);
	}
}

/// on closing the player
static int on_close(GtkWidget *window) {
	// Free the allocated memory
	isWindowOpened = FALSE;
	sleep(1);

	gint nrOfLines;
	tabClicked = get_int_value_from_conf(pathToConfigDmelody, "lastTabClicked");

	if (tabClicked == 1) {
		nrOfLines = get_number_of_lines(pathToPlaylist1);
	}
	else if (tabClicked == 2) {
		nrOfLines = get_number_of_lines(pathToPlaylist2);
	}
	else {
		nrOfLines = get_number_of_lines(pathToPlaylist3);
	}

	if (nrOfLines > 1) {
		g_print("Freeing memory\n");
		for (int i = 0; allNumbers[i] != NULL; i++) {
			free(allNumbers[i]);
			free(allArtists[i]);
			free(allTitles[i]);
			free(allAlbums[i]);
			free(allDurations[i]);

			allNumbers[i] = NULL;
			allArtists[i] = NULL;
			allTitles[i] = NULL;
			allAlbums[i] = NULL;
			allDurations[i] = NULL;
		}

		//free(strs_text);
		free(allNumbers);
		free(allArtists);
		free(allTitles);
		free(allAlbums);
		free(allDurations);

		allNumbers = NULL;
		allArtists = NULL;
		allTitles = NULL;
		allAlbums = NULL;
		allDurations = NULL;
	}

	gtk_expression_watch_unwatch(watchMaximized);
	gtk_expression_unref(expressionMaximized);
	gtk_expression_watch_unwatch(watchWidth);
	gtk_expression_unref(expressionWidth);
	gtk_expression_watch_unwatch(watchHeight);
	gtk_expression_unref(expressionHeight);
	gtk_expression_unref(expressionStateSwitch);
	gtk_expression_watch_unwatch(watchRepeatSwitch);

	// kill mpv if running
	if (pid != 0) {
		//kill(pid, SIGTERM);
		g_spawn_close_pid(pid);
		send_mpv_cmd("/tmp/mpvsocket", "quit\n");
	}

	gtk_window_close(GTK_WINDOW(window));
	return false;
}

/// quit action
static void quit_cb() {
	gtk_popover_popdown(GTK_POPOVER(menuPop));
	gtk_widget_set_can_target(browserScrolledWindow, TRUE);
}

/// actions structure
const GActionEntry app_actions[] = {
	{
		.name = "opendir",
		.activate = opendir_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "addtopl1",
		.activate = addtopl1_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "addtopl2",
		.activate = addtopl2_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "addtopl3",
		.activate = addtopl3_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "replacepl1",
		.activate = replacepl1_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "replacepl2",
		.activate = replacepl2_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "replacepl3",
		.activate = replacepl3_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	},
	{
		.name = "quit",
		.activate = quit_cb,
		.parameter_type = NULL,
		.state = NULL,
		.change_state = NULL,
		.padding = {0, 0, 0}
	}
};

// this runs on click on selected item
static void set_can_target_false() {
	gtk_popover_popup(GTK_POPOVER(menuPop));
	gtk_widget_set_can_target(browserScrolledWindow, FALSE);
}

/// creating labels to fill up the listview
static void setup_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;
	GtkWidget *label;
	label = gtk_label_new(NULL);

	// defining the size of text in label
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(17 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(label, NULL);
	pango_layout_set_attributes(layout, attr_list);

	gtk_label_set_attributes(GTK_LABEL(label), attr_list);

	GtkWidget *icon;
	icon = gtk_image_new_from_icon_name("folder");
	gtk_image_set_icon_size(GTK_IMAGE(icon), GTK_ICON_SIZE_NORMAL);

	GtkWidget *box;
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_prepend(GTK_BOX(box), icon);
	gtk_box_append(GTK_BOX(box), label);

	gtk_list_item_set_child(list_item, box);

	// freing resources
	g_object_unref(layout);
	//pango_attribute_destroy(attr);
	pango_attr_list_unref(attr_list);
}

/// binding the items to the list, this function runs in a loop
static void bind_listitem(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory; // Marking the parameter as unused

	// Get the GtkBox widget
	GtkWidget *box;
	box = gtk_list_item_get_child(list_item);

	// Find the GtkImage widget within the GtkBox
	GtkWidget *icon;
	icon = gtk_widget_get_first_child(box);

	// Find the next child after GtkImage whoch is a label 
	GtkWidget *label;
	label = gtk_widget_get_next_sibling(icon);

	// gets the strings from GtkStringList *myStrings in startup function
	GtkStringObject *obj = gtk_list_item_get_item(list_item);
	const char *strings = gtk_string_object_get_string(GTK_STRING_OBJECT(obj));

	// sets label text to strings one at a time
	gtk_label_set_label(GTK_LABEL(label), strings);
}

//////////////////////////// setting up columns
static void setup_listitem_nums(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;
	GtkWidget *number;
	number = gtk_label_new(NULL);
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(17 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(number, NULL);
	pango_layout_set_attributes(layout, attr_list);
	gtk_label_set_attributes(GTK_LABEL(number), attr_list);
	gtk_list_item_set_child(list_item, number);
	g_object_unref(layout);
	pango_attr_list_unref(attr_list);
}

static void bind_listitem_nums(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;
	GtkWidget *number;
	number = gtk_list_item_get_child(list_item);
	guint listitemPos = gtk_list_item_get_position(list_item);
	const gchar *s_numbers = gtk_string_list_get_string(numbers, listitemPos);
	gtk_label_set_label(GTK_LABEL(number), s_numbers);
}

static void bind_listitem_arts(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;
	GtkWidget *artist;
	artist = gtk_list_item_get_child(list_item);
	gtk_label_set_ellipsize(GTK_LABEL(artist), PANGO_ELLIPSIZE_END);
	guint listitemPos = gtk_list_item_get_position(list_item);
	const gchar *s_artists = gtk_string_list_get_string(artists, listitemPos);
	gtk_label_set_label(GTK_LABEL(artist), s_artists);
}

static void bind_listitem_ttls(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;
	GtkWidget *title;
	title = gtk_list_item_get_child(list_item);
	gtk_label_set_ellipsize(GTK_LABEL(title), PANGO_ELLIPSIZE_END);
	guint listitemPos = gtk_list_item_get_position(list_item);
	const gchar *s_titles = gtk_string_list_get_string(titles, listitemPos);
	gtk_label_set_label(GTK_LABEL(title), s_titles);
}

static void bind_listitem_albums(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;
	GtkWidget *album;
	album = gtk_list_item_get_child(list_item);
	gtk_label_set_ellipsize(GTK_LABEL(album), PANGO_ELLIPSIZE_END);
	guint listitemPos = gtk_list_item_get_position(list_item);
	const gchar *s_albums = gtk_string_list_get_string(albums, listitemPos);
	gtk_label_set_label(GTK_LABEL(album), s_albums);
}

static void bind_listitem_duration(GtkListItemFactory *factory, GtkListItem *list_item) {
	(void)factory;
	GtkWidget *duration;
	duration = gtk_list_item_get_child(list_item);
	gtk_label_set_ellipsize(GTK_LABEL(duration), PANGO_ELLIPSIZE_END);
	guint listitemPos = gtk_list_item_get_position(list_item);
	const gchar *s_durations = gtk_string_list_get_string(durations, listitemPos);
	gtk_label_set_label(GTK_LABEL(duration), s_durations);
}

/// click home button refreshes directory list
static void refresh_dirlist(GtkEntryBuffer *pathBuff) {
	openDirState = 0;
	addToPlaylistState = 0;
	addToPlaylistState2 = 0;
	memset(fullPathToSelDir, 0, sizeof(fullPathToSelDir));

	// add back 'Open directory' action if it was remover
	g_menu_remove(menu, 0);
	g_menu_prepend(menu, "Open directory", "app.opendir");

	const gchar *buffText = gtk_entry_buffer_get_text(pathBuff);
	gchar musicPathOverwrite[strlen(buffText) + 11];
	snprintf(musicPathOverwrite, sizeof(musicPathOverwrite), "musicpath=%s", buffText);
	replacenthline(pathToConfigDmelody, 7, musicPathOverwrite);

	// repopulate dirs and path to Music dir
	const gchar *musicPath = get_char_value_from_conf(pathToConfigDmelody, "musicpath");
	populate_dirs(musicPath);

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
}

/// activate signal handle
static void activate(GApplication *application, GApplicationCommandLine *cmdline) {
	(void)application;

	gint argc;
	gchar **argv;

	argv = g_application_command_line_get_arguments(cmdline, &argc);

	//////////////////////////////////////// if command line arguments provided
	if (appstate == 1 && argv[1] != NULL && argc > 1) {
		if (argc > 2) {
			printf("Too many arguments\n");
			return;
		}
		// help
		if (argv[1] != NULL && strcmp(argv[1], "--help") == 0) {
			g_application_command_line_print(cmdline, "DMelody 1.0\n"
			"Actions:\n"
				"	--help			Show help.\n"
				"	--previous		Play previous track.\n"
				"	--toggle-pause		Play if paused and pause if playing.\n"
				"	--next			Play next track.\n"
				"	--quit			Quit DNelody.\n"
				"	--version		Show version information.\n");
				return;
		}
		// previous
		else if (argv[1] != NULL && strcmp(argv[1], "--previous") == 0) {
			previous_trackF();
			return;
		}
		// play/pause
		else if (argv[1] != NULL && strcmp(argv[1], "--toggle-pause") == 0) {
			play_pauseF(play_pause);
			return;
		}
		// next
		else if (argv[1] != NULL && strcmp(argv[1], "--next") == 0) {
			next_trackF();
			return;
		}
		// quit
		else if (argv[1] != NULL && strcmp(argv[1], "--quit") == 0) {
			g_application_command_line_print(cmdline, "Good luck!\n");
			gtk_window_close(GTK_WINDOW(dmelodyWindow));
			return;
		}
		// version
		else if (argv[1] != NULL && strcmp(argv[1], "--version") == 0) {
			g_application_command_line_print(cmdline, "%s\n", VERSION);
			return;
		}
		// invalid argument
		else if (argv[1] != NULL) {
			g_application_command_line_print(cmdline, "Please see --help for available options.\n");
			return;
		}

		g_strfreev(argv);
 		g_object_unref(cmdline);
 		g_application_command_line_set_exit_status(cmdline, 0);

		return;
	}

	////////////////////////////////////// only one instance running allowed
	else if (appstate == 1 && argv[1] == NULL && argc == 1) { // handle arguments
		g_application_command_line_print(cmdline, "already running, only one instance allowed\n");
		return;
	}

	///////////////////////////////////// first time launch starting main application
	else {
		// starting the main application window
		g_print("Welcome to DMelody!\n");
		gtk_window_present(GTK_WINDOW(dmelodyWindow));

		//////////////////////////////// reading settings ////////////////////////////////////////
		gint nrOfLines;
		tabClicked = get_int_value_from_conf(pathToConfigDmelody, "lastTabClicked");

		if (tabClicked == 1) {
			nrOfLines = get_number_of_lines(pathToPlaylist1);
		}
		else if (tabClicked == 2) {
			nrOfLines = get_number_of_lines(pathToPlaylist2);
		}
		else {
			nrOfLines = get_number_of_lines(pathToPlaylist3);
		}

		// starts autoplay on player startup
		if (get_int_value_from_conf(pathToConfigDmelody, "autoplay") == 1 && nrOfLines >= 1) {
			g_print("Autoplay on\n");
			g_signal_emit_by_name(browserList, "activate");
		}

		// start minimized
		if (get_int_value_from_conf(pathToConfigDmelody, "startminimized") == 1) {
			g_print("Minimizing\n");
			gtk_window_minimize(GTK_WINDOW(dmelodyWindow));
		}

		// autostart control bar
		if (get_int_value_from_conf(pathToConfigDmelody, "controlbar") == 1 && nrOfLines >= 1) {
			g_print("Show control bar on\n");
			g_signal_emit_by_name(showControls, "clicked");
		}
	}

	// sets flag that app is already running
	appstate = 1;
}

/// startup signal handle
static void startup(GtkApplication *app) {
	///////////////////////////// setting up all system variables /////////////////////////////////
	// expression looks au an abstract switch property "state"
	expressionStateSwitch = gtk_property_expression_new(GTK_TYPE_SWITCH, NULL, "state");

	// deifning HOME
	HOME = getenv("HOME");

	// setting config files
	snprintf((char *)pathToPlaylist1, 777, "%s%s", HOME, configPlaylist1);
	snprintf((char *)pathToPlaylist2, 777, "%s%s", HOME, configPlaylist2);
	snprintf((char *)pathToPlaylist3, 777, "%s%s", HOME, configPlaylist3);
	snprintf((char *)pathToConfigDmelody, 777, "%s%s", HOME, configDmelody);
	snprintf((char *)pathToConfigDmelodyDir, 777, "%s%s", HOME, configDmelodyDir);

	snprintf((char *)pathToConfigDmelodyDirNumbersFile, 777, "%s%s", pathToConfigDmelodyDir, nums);
	snprintf((char *)pathToConfigDmelodyDirArtistsFile, 777, "%s%s", pathToConfigDmelodyDir, arts);
	snprintf((char *)pathToConfigDmelodyDirTitlesFile, 777, "%s%s", pathToConfigDmelodyDir, ttls);
	snprintf((char *)pathToConfigDmelodyDirAlbumsFile, 777, "%s%s", pathToConfigDmelodyDir, albms);
	snprintf((char *)pathToConfigDmelodyDirDurationsFIle, 777, "%s%s", pathToConfigDmelodyDir, durs);

	snprintf((char *)pathToConfigDmelodyDirNumbersFile2, 777, "%s%s", pathToConfigDmelodyDir, nums2);
	snprintf((char *)pathToConfigDmelodyDirArtistsFile2, 777, "%s%s", pathToConfigDmelodyDir, arts2);
	snprintf((char *)pathToConfigDmelodyDirTitlesFile2, 777, "%s%s", pathToConfigDmelodyDir, ttls2);
	snprintf((char *)pathToConfigDmelodyDirAlbumsFile2, 777, "%s%s", pathToConfigDmelodyDir, albms2);
	snprintf((char *)pathToConfigDmelodyDirDurationsFIle2, 777, "%s%s", pathToConfigDmelodyDir,
																							durs2);

	snprintf((char *)pathToConfigDmelodyDirNumbersFile3, 777, "%s%s", pathToConfigDmelodyDir, nums3);
	snprintf((char *)pathToConfigDmelodyDirArtistsFile3, 777, "%s%s", pathToConfigDmelodyDir, arts3);
	snprintf((char *)pathToConfigDmelodyDirTitlesFile3, 777, "%s%s", pathToConfigDmelodyDir, ttls3);
	snprintf((char *)pathToConfigDmelodyDirAlbumsFile3, 777, "%s%s", pathToConfigDmelodyDir, albms3);
	snprintf((char *)pathToConfigDmelodyDirDurationsFIle3, 777, "%s%s", pathToConfigDmelodyDir,
																							durs3);

	// setting exifpath
	FILE *exiftool = fopen("/usr/local/share/dmelody/exiftool/exiftool", "r");
	if (exiftool == NULL) {
		// programs was not installed to /usr/local/ so runs from current dir
		perror("/usr/local/share/dmelody/exiftool/exiftool");
		g_print("Setting exif path to a local path\n");
		snprintf(EXIFARTIST, 256, "usr/local/share/dmelody/exiftool/exiftool -f -Artist");
		snprintf(EXIFTITLE, 256, "usr/local/share/dmelody/exiftool/exiftool -f -Title");
		snprintf(EXIFALBUM, 256, "usr/local/share/dmelody/exiftool/exiftool -f -Album");
		snprintf(EXIFDURATION, 256, "usr/local/share/dmelody/exiftool/exiftool -f -Duration");
		snprintf(EXIFFILENAME, 256, "usr/local/share/dmelody/exiftool/exiftool -f -FileName");

		snprintf(defaultEmptyCover, 777,
							"usr/local/share/icons/hicolor/96x96/apps/dmelody-image.png");
	}
	else {
		// programs was installed to /usr/local/
		snprintf(EXIFARTIST, 256, "/usr/local/share/dmelody/exiftool/exiftool -f -Artist");
		snprintf(EXIFTITLE, 256, "/usr/local/share/dmelody/exiftool/exiftool -f -Title");
		snprintf(EXIFALBUM, 256, "/usr/local/share/dmelody/exiftool/exiftool -f -Album");
		snprintf(EXIFDURATION, 256, "/usr/local/share/dmelody/exiftool/exiftool -f -Duration");
		snprintf(EXIFFILENAME, 256, "/usr/local/share/dmelody/exiftool/exiftool -f -FileName");

		snprintf(defaultEmptyCover, 777,
								"/usr/local/share/icons/hicolor/96x96/apps/dmelody-image.png");
		fclose(exiftool);
	}
	// get number of lines in the generated playlist to determine whether any tracks were added
	gint nrOfLines = get_number_of_lines(pathToPlaylist1);
	if (nrOfLines < 1) {
		nrOfLines = get_number_of_lines(pathToPlaylist2);
	}
	if (nrOfLines < 1) {
		nrOfLines = get_number_of_lines(pathToPlaylist3);
	}
	// getting cover imaget from conf files
	snprintf(pathToPlaylist1Cover, 777, get_char_value_from_conf(pathToConfigDmelody,
																			"playlist1cover"));
	snprintf(pathToPlaylist2Cover, 777, get_char_value_from_conf(pathToConfigDmelody,
																			"playlist2cover"));
	snprintf(pathToPlaylist3Cover, 777, get_char_value_from_conf(pathToConfigDmelody,
																			"playlist3cover"));
	///////////////////////// pupulating dir names and listitemfactory ////////////////////////////
	// Create items in the menu
	GMenuItem *item1 = g_menu_item_new("Open directory", "app.opendir");
	GMenuItem *item2 = g_menu_item_new("Add to Playlist 1", "app.addtopl1");
	GMenuItem *item3 = g_menu_item_new("Add to Playlist 2", "app.addtopl2");
	GMenuItem *item4 = g_menu_item_new("Add to Playlist 3", "app.addtopl3");
	GMenuItem *item5 = g_menu_item_new("Replace Playlist 1", "app.replacepl1");
	GMenuItem *item6 = g_menu_item_new("Replace Playlist 2", "app.replacepl2");
	GMenuItem *item7 = g_menu_item_new("Replace Playlist 3", "app.replacepl3");
	GMenuItem *item8 = g_menu_item_new("Close", "app.quit");

	// menu model Append items to the menu
	menu = g_menu_new();
	g_menu_append_item(menu, item1);
	g_menu_append_item(menu, item2);
	g_menu_append_item(menu, item3);
	g_menu_append_item(menu, item4);
	g_menu_append_item(menu, item5);
	g_menu_append_item(menu, item6);
	g_menu_append_item(menu, item7);
	g_menu_append_item(menu, item8);

	// GMenuModel *menuModel Create a GMenuModel from the GMenu
	menuModel = G_MENU_MODEL(menu);

	const gchar *musicPath = get_char_value_from_conf(pathToConfigDmelody, "musicpath");
	populate_dirs(musicPath);

	directories = gtk_string_list_new((const char * const *)dirs);

	// Free allocated memory
	for (size_t i = 0; dirs[i] != NULL; i++) {
		free(dirs[i]);
	}
	free(dirs);
	free((void *)musicPath);

	// creates a list model from the object of string objects
	dirsListModel = G_LIST_MODEL(directories);

	// creats a list item factory to setup and bind the string object to the list view
	GtkListItemFactory *gtkListItemFactoryDirs;
	gtkListItemFactoryDirs = gtk_signal_list_item_factory_new();

	// callbacks (funtions) to run on signals that widgets emit
	g_signal_connect(gtkListItemFactoryDirs, "setup", G_CALLBACK(setup_listitem), NULL);
	g_signal_connect(gtkListItemFactoryDirs, "bind", G_CALLBACK(bind_listitem), NULL);

	// defining how selection of items behaves
	singleSelectionDirs = gtk_single_selection_new(G_LIST_MODEL(dirsListModel));

	GtkSelectionModel *selectionModelDirs;
	selectionModelDirs = GTK_SELECTION_MODEL(singleSelectionDirs);

	GtkEntryBuffer *pathBuff;
	pathBuff = gtk_entry_buffer_new(get_char_value_from_conf(pathToConfigDmelody, "musicpath"), -1);
	////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////// WIDGEtS /////////////////////////////////////////////

	// popup menu for file browser below the cover art
	dmelodyWindow = gtk_application_window_new(GTK_APPLICATION(app));
	gtk_window_set_icon_name(GTK_WINDOW(dmelodyWindow), "dmelody");
	gtk_window_set_title(GTK_WINDOW(dmelodyWindow), VERSION);
	gtk_window_set_default_size(GTK_WINDOW(dmelodyWindow),
							atoi(get_char_value_from_conf(pathToConfigDmelody, "width")),
							atoi(get_char_value_from_conf(pathToConfigDmelody, "height")));

	g_signal_connect_swapped(dmelodyWindow, "close-request", G_CALLBACK(on_close), dmelodyWindow);

	expressionWidth = gtk_property_expression_new(GTK_TYPE_WINDOW, NULL, "default-width");
	watchWidth = gtk_expression_watch(expressionWidth, dmelodyWindow, notify, NULL, NULL);

	expressionHeight = gtk_property_expression_new(GTK_TYPE_WINDOW, NULL, "default-height");
	watchHeight = gtk_expression_watch(expressionHeight, dmelodyWindow, notify, NULL, NULL);

	expressionMaximized = gtk_property_expression_new(GTK_TYPE_WINDOW, NULL, "maximized");
	watchMaximized = gtk_expression_watch(expressionMaximized, dmelodyWindow, notif_max, NULL, NULL);
	// **************** cover | home button | entry music path | dir browser | popup ***************
	// cover art picture
	cover = gtk_image_new();
	gtk_widget_set_size_request(cover, 300, 300);
	gtk_image_set_pixel_size(GTK_IMAGE(cover), 300);
	gtk_widget_set_hexpand(cover, FALSE);
	gtk_widget_set_vexpand(cover, FALSE);
	gtk_widget_set_hexpand_set(cover, FALSE);
	gtk_widget_set_vexpand_set(cover, FALSE);

	// set cover art
	if (nrOfLines >= 1) {
		tabClicked = get_int_value_from_conf(pathToConfigDmelody, "lastTabClicked");
		if (tabClicked == 1) {
			// if last opened tab was playlist 1
			gtk_image_set_from_file(GTK_IMAGE(cover), pathToPlaylist1Cover);
			
		}
		else if (tabClicked == 2) {
			// if last opened tab was playlist 2
			gtk_image_set_from_file(GTK_IMAGE(cover), pathToPlaylist2Cover);
		}
		else {
			// if last opened tab was playlist 3
			gtk_image_set_from_file(GTK_IMAGE(cover), pathToPlaylist3Cover);
		}
	}
	else {
		// if it's the first launch of the player
		gtk_image_set_from_file(GTK_IMAGE(cover), defaultEmptyCover);
	}

	// home button for Music path
	GtkWidget *home;
	home = gtk_button_new_from_icon_name("go-home-symbolic");
	g_signal_connect_swapped(home, "clicked", G_CALLBACK(refresh_dirlist), pathBuff);
	// entry to display the path to Music
	GtkWidget *pathMusic;
	pathMusic = gtk_entry_new_with_buffer(pathBuff);
	gtk_widget_set_size_request(pathMusic, 262, 0);
	// home and entry Music path box
	GtkWidget *home_path_hor_box;
	home_path_hor_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_prepend(GTK_BOX(home_path_hor_box), home);
	gtk_box_append(GTK_BOX(home_path_hor_box), pathMusic);

	// folder browser
	GtkWidget *browser;
	browser = gtk_list_view_new(selectionModelDirs, gtkListItemFactoryDirs);
	gtk_list_view_set_single_click_activate(GTK_LIST_VIEW(browser), TRUE);
	g_signal_connect_swapped(browser, "activate", G_CALLBACK(set_can_target_false), NULL);

	// folder browser sctolling window
	browserScrolledWindow = gtk_scrolled_window_new();
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(browserScrolledWindow), browser);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(browserScrolledWindow), 250);
	gtk_widget_set_valign(browserScrolledWindow, GTK_ALIGN_END);

	// popup menu when clicking on a directory in directory browser
	menuPop = gtk_popover_menu_new_from_model(menuModel);
	gtk_widget_set_size_request(menuPop, 290, 100);
	gtk_popover_set_offset(GTK_POPOVER(menuPop), 0, -500);
	g_signal_connect_swapped(menuPop, "closed", G_CALLBACK(quit_cb), NULL);
	// time elpsed / remaining
	timeElapsed = gtk_label_new(" ---- / ---- ");
	gtk_widget_set_valign(timeElapsed, GTK_ALIGN_CENTER);
	gtk_widget_set_halign(timeElapsed, GTK_ALIGN_CENTER);
	gtk_widget_set_vexpand(timeElapsed, FALSE);
	gtk_widget_set_hexpand(timeElapsed, FALSE);

	// frame playlists box
	GtkWidget *time_frame;
	time_frame = gtk_frame_new(NULL);
	gtk_widget_set_hexpand(time_frame, FALSE);
	gtk_widget_set_vexpand(time_frame, FALSE);
	gtk_widget_set_hexpand_set(time_frame, FALSE);
	gtk_widget_set_vexpand_set(time_frame, FALSE);
	gtk_frame_set_child(GTK_FRAME(time_frame), timeElapsed);

	// customizing the style, text size of the GtkLabel Widget
	PangoAttrList *attr_list_Playlists = pango_attr_list_new();
	PangoAttribute *attrPlaylists = pango_attr_size_new_absolute(17 * PANGO_SCALE);
	pango_attr_list_insert(attr_list_Playlists, attrPlaylists);
	PangoLayout *layoutPlaylists = gtk_widget_create_pango_layout(timeElapsed, NULL);
	pango_layout_set_attributes(layoutPlaylists, attr_list_Playlists);
	gtk_label_set_attributes(GTK_LABEL(timeElapsed), attr_list_Playlists);

	// cover | home, path | browser scroll | popup
	GtkWidget *cov_hpbox_browscroll_pop;
	cov_hpbox_browscroll_pop = gtk_box_new(GTK_ORIENTATION_VERTICAL, 7);
	gtk_box_prepend(GTK_BOX(cov_hpbox_browscroll_pop), cover);
	gtk_box_append(GTK_BOX(cov_hpbox_browscroll_pop), home_path_hor_box);
	gtk_box_append(GTK_BOX(cov_hpbox_browscroll_pop), browserScrolledWindow);
	gtk_box_append(GTK_BOX(cov_hpbox_browscroll_pop), time_frame);
	gtk_box_append(GTK_BOX(cov_hpbox_browscroll_pop), menuPop);
	gtk_widget_set_halign(cov_hpbox_browscroll_pop, GTK_ALIGN_START);
	gtk_widget_set_hexpand(cov_hpbox_browscroll_pop, FALSE);
	gtk_widget_set_vexpand(cov_hpbox_browscroll_pop, FALSE);
	gtk_widget_set_hexpand_set(cov_hpbox_browscroll_pop, FALSE);
	gtk_widget_set_vexpand_set(cov_hpbox_browscroll_pop, FALSE);
	// *********************************************************************************************
	// ********************  Playlist 1 | Playlist 2 | Playlist 3 **********************************
	// Playlist 1 Separator
	GtkWidget *playlist1Sep;
	playlist1Sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	// Playlist 1
	playlist1 = gtk_label_new(NULL);
	gtk_label_set_label(GTK_LABEL(playlist1),
						get_char_value_from_conf(pathToConfigDmelody, "playlist1name"));
	gtk_label_set_ellipsize(GTK_LABEL(playlist1), PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand(playlist1, FALSE);
	gtk_widget_set_vexpand(playlist1, FALSE);
	gtk_widget_set_hexpand_set(playlist1, FALSE);
	gtk_widget_set_vexpand_set(playlist1, FALSE);
	GtkGesture *playlist1ges;
	playlist1ges = gtk_gesture_click_new();
	GtkGestureSingle *playlist1gesClick;
	playlist1gesClick = GTK_GESTURE_SINGLE(playlist1ges);
	gtk_gesture_single_set_button(playlist1gesClick, 1);
	GtkEventController *playlist1gesLClick;
	playlist1gesLClick = GTK_EVENT_CONTROLLER(playlist1ges);
	gtk_event_controller_set_propagation_phase(playlist1gesLClick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(playlist1, playlist1gesLClick);
	g_signal_connect_swapped(playlist1gesClick, "pressed", G_CALLBACK(on_playlist1_l_click), NULL);
	// Playlist 2 Separator
	GtkWidget *playlist2Sep;
	playlist2Sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	// Playlist 2
	playlist2 = gtk_label_new(NULL);
	gtk_label_set_label(GTK_LABEL(playlist2),
						get_char_value_from_conf(pathToConfigDmelody, "playlist2name"));
	gtk_label_set_ellipsize(GTK_LABEL(playlist2), PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand(playlist2, FALSE);
	gtk_widget_set_vexpand(playlist2, FALSE);
	gtk_widget_set_hexpand_set(playlist2, FALSE);
	gtk_widget_set_vexpand_set(playlist2, FALSE);
	GtkGesture *playlist2ges;
	playlist2ges = gtk_gesture_click_new();
	GtkGestureSingle *playlist2gesClick;
	playlist2gesClick = GTK_GESTURE_SINGLE(playlist2ges);
	gtk_gesture_single_set_button(playlist2gesClick, 1);
	GtkEventController *playlist2gesLClick;
	playlist2gesLClick = GTK_EVENT_CONTROLLER(playlist2ges);
	gtk_event_controller_set_propagation_phase(playlist2gesLClick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(playlist2, playlist2gesLClick);
	g_signal_connect_swapped(playlist2gesClick, "pressed", G_CALLBACK(on_playlist2_l_click), NULL);
	// Playlist 2 Separator
	GtkWidget *playlist3Sep;
	playlist3Sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	// Playlist 3
	playlist3 = gtk_label_new(NULL);
	gtk_label_set_label(GTK_LABEL(playlist3),
						get_char_value_from_conf(pathToConfigDmelody, "playlist3name"));
	gtk_label_set_ellipsize(GTK_LABEL(playlist3), PANGO_ELLIPSIZE_END);
	gtk_widget_set_hexpand(playlist3, FALSE);
	gtk_widget_set_vexpand(playlist3, FALSE);
	gtk_widget_set_hexpand_set(playlist3, FALSE);
	gtk_widget_set_vexpand_set(playlist3, FALSE);
	GtkGesture *playlist3ges;
	playlist3ges = gtk_gesture_click_new();
	GtkGestureSingle *playlist3gesClick;
	playlist3gesClick = GTK_GESTURE_SINGLE(playlist3ges);
	gtk_gesture_single_set_button(playlist3gesClick, 1);
	GtkEventController *playlist3gesLClick;
	playlist3gesLClick = GTK_EVENT_CONTROLLER(playlist3ges);
	gtk_event_controller_set_propagation_phase(playlist3gesLClick, GTK_PHASE_CAPTURE);
	gtk_widget_add_controller(playlist3, playlist3gesLClick);
	g_signal_connect_swapped(playlist3gesClick, "pressed", G_CALLBACK(on_playlist3_l_click), NULL);
	// Playlist 2 Separator
	GtkWidget *playlist4Sep;
	playlist4Sep = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
	// waiting popup
	GtkWidget *waitingLabel;
	waitingLabel = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(waitingLabel),
						"<span size=\"20pt\" weight=\"bold\">Please Wait...</span>");
	waitingPopup = gtk_popover_new();
	gtk_popover_set_offset(GTK_POPOVER(waitingPopup), 0, 300);
	gtk_popover_set_autohide(GTK_POPOVER(waitingPopup), FALSE);
	gtk_popover_set_has_arrow(GTK_POPOVER(waitingPopup), FALSE);
	gtk_popover_set_child(GTK_POPOVER(waitingPopup), waitingLabel);
	// Playlist 1 | Playlist 2 | Playlist 3
	GtkWidget *lab1_lab2_lab3_box;
	lab1_lab2_lab3_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 70);
	gtk_box_set_homogeneous(GTK_BOX(lab1_lab2_lab3_box), TRUE);
	gtk_box_prepend(GTK_BOX(lab1_lab2_lab3_box), playlist1Sep);
	gtk_box_append(GTK_BOX(lab1_lab2_lab3_box), playlist1);
	gtk_box_append(GTK_BOX(lab1_lab2_lab3_box), playlist2Sep);
	gtk_box_append(GTK_BOX(lab1_lab2_lab3_box), playlist2);
	gtk_box_append(GTK_BOX(lab1_lab2_lab3_box), playlist3Sep);
	gtk_box_append(GTK_BOX(lab1_lab2_lab3_box), playlist3);
	gtk_box_append(GTK_BOX(lab1_lab2_lab3_box), playlist4Sep);
	gtk_box_append(GTK_BOX(lab1_lab2_lab3_box), waitingPopup);
	gtk_widget_set_hexpand(lab1_lab2_lab3_box, FALSE);
	gtk_widget_set_vexpand(lab1_lab2_lab3_box, FALSE);
	gtk_widget_set_hexpand_set(lab1_lab2_lab3_box, FALSE);
	gtk_widget_set_vexpand_set(lab1_lab2_lab3_box, FALSE);
	// *********************************************************************************************
	// ********** a;; playlists | No | Artist | Title | Album | Duration | Playlist view ***********
	// check if any directories have been added to playlist
	if (nrOfLines >= 1) {
		tabClicked = get_int_value_from_conf(pathToConfigDmelody, "lastTabClicked");
		if (tabClicked == 1) {
			// defining the size of text in label
			PangoAttrList *attr_list = pango_attr_list_new();
			PangoAttribute *attr = pango_attr_size_new_absolute(20 * PANGO_SCALE);
			pango_attr_list_insert(attr_list, attr);
			PangoAttribute *weight_attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
			pango_attr_list_insert(attr_list, weight_attr);
			PangoLayout *layout = gtk_widget_create_pango_layout(playlist1, NULL);
			pango_layout_set_attributes(layout, attr_list);

			gtk_label_set_attributes(GTK_LABEL(playlist1), attr_list);
			gtk_label_set_attributes(GTK_LABEL(playlist2), NULL);
			gtk_label_set_attributes(GTK_LABEL(playlist3), NULL);

			g_object_unref(layout);
			pango_attr_list_unref(attr_list);

			populate_allNumbers(pathToConfigDmelodyDirNumbersFile);
			populate_allArtists(pathToConfigDmelodyDirArtistsFile);
			populate_allTitles(pathToConfigDmelodyDirTitlesFile);
			populate_allAlbums(pathToConfigDmelodyDirAlbumsFile);
			populate_allDurations(pathToConfigDmelodyDirDurationsFIle);

			numbers = gtk_string_list_new((const char * const *)allNumbers);
			artists = gtk_string_list_new((const char * const *)allArtists);
			titles = gtk_string_list_new((const char * const *)allTitles);
			albums = gtk_string_list_new((const char * const *)allAlbums);
			durations = gtk_string_list_new((const char * const *)allDurations);
		}
		else if (tabClicked == 2) {
			// defining the size of text in label
			PangoAttrList *attr_list = pango_attr_list_new();
			PangoAttribute *attr = pango_attr_size_new_absolute(20 * PANGO_SCALE);
			pango_attr_list_insert(attr_list, attr);
			PangoAttribute *weight_attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
			pango_attr_list_insert(attr_list, weight_attr);
			PangoLayout *layout = gtk_widget_create_pango_layout(playlist2, NULL);
			pango_layout_set_attributes(layout, attr_list);

			gtk_label_set_attributes(GTK_LABEL(playlist1), NULL);
			gtk_label_set_attributes(GTK_LABEL(playlist2), attr_list);
			gtk_label_set_attributes(GTK_LABEL(playlist3), NULL);

			g_object_unref(layout);
			pango_attr_list_unref(attr_list);

			populate_allNumbers(pathToConfigDmelodyDirNumbersFile2);
			populate_allArtists(pathToConfigDmelodyDirArtistsFile2);
			populate_allTitles(pathToConfigDmelodyDirTitlesFile2);
			populate_allAlbums(pathToConfigDmelodyDirAlbumsFile2);
			populate_allDurations(pathToConfigDmelodyDirDurationsFIle2);

			numbers = gtk_string_list_new((const char * const *)allNumbers);
			artists = gtk_string_list_new((const char * const *)allArtists);
			titles = gtk_string_list_new((const char * const *)allTitles);
			albums = gtk_string_list_new((const char * const *)allAlbums);
			durations = gtk_string_list_new((const char * const *)allDurations);
		}
		else {
			// defining the size of text in label
			PangoAttrList *attr_list = pango_attr_list_new();
			PangoAttribute *attr = pango_attr_size_new_absolute(20 * PANGO_SCALE);
			pango_attr_list_insert(attr_list, attr);
			PangoAttribute *weight_attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
			pango_attr_list_insert(attr_list, weight_attr);
			PangoLayout *layout = gtk_widget_create_pango_layout(playlist3, NULL);
			pango_layout_set_attributes(layout, attr_list);

			gtk_label_set_attributes(GTK_LABEL(playlist1), NULL);
			gtk_label_set_attributes(GTK_LABEL(playlist2), NULL);
			gtk_label_set_attributes(GTK_LABEL(playlist3), attr_list);

			g_object_unref(layout);
			pango_attr_list_unref(attr_list);

			populate_allNumbers(pathToConfigDmelodyDirNumbersFile3);
			populate_allArtists(pathToConfigDmelodyDirArtistsFile3);
			populate_allTitles(pathToConfigDmelodyDirTitlesFile3);
			populate_allAlbums(pathToConfigDmelodyDirAlbumsFile3);
			populate_allDurations(pathToConfigDmelodyDirDurationsFIle3);

			numbers = gtk_string_list_new((const char * const *)allNumbers);
			artists = gtk_string_list_new((const char * const *)allArtists);
			titles = gtk_string_list_new((const char * const *)allTitles);
			albums = gtk_string_list_new((const char * const *)allAlbums);
			durations = gtk_string_list_new((const char * const *)allDurations);
		}
	}
	else {
		gchar *nums[] = { "Hi!", NULL };
		numbers = gtk_string_list_new((const char * const *)nums);

		gchar *arts[] = { getenv("USER"), NULL };
		artists = gtk_string_list_new((const char * const *)arts);

		gchar *ttls[] = { "Welcomw to DMelody!", NULL };
		titles = gtk_string_list_new((const char * const *)ttls);

		gchar *albms[] = { "I hope you like it", NULL };
		albums = gtk_string_list_new((const char * const *)albms);

		gchar *durs[] = { ":)", NULL };
		durations = gtk_string_list_new((const char * const *)durs);
	}

	ListModelNumbers = G_LIST_MODEL(numbers);
	GtkListItemFactory *gtkListItemFactoryNums;
	gtkListItemFactoryNums = gtk_signal_list_item_factory_new();
	g_signal_connect(gtkListItemFactoryNums, "setup", G_CALLBACK(setup_listitem_nums), NULL);
	g_signal_connect(gtkListItemFactoryNums, "bind", G_CALLBACK(bind_listitem_nums), NULL);
	singleSelectionNums = gtk_single_selection_new(G_LIST_MODEL(ListModelNumbers));
	selectionModel = GTK_SELECTION_MODEL(singleSelectionNums);
	columnViewNums = gtk_column_view_column_new("  No", gtkListItemFactoryNums);
	gtk_column_view_column_set_fixed_width(GTK_COLUMN_VIEW_COLUMN(columnViewNums), 44);
	gtk_column_view_column_set_expand(GTK_COLUMN_VIEW_COLUMN(columnViewNums), FALSE);

	GtkListItemFactory *gtkListItemFactoryArtists;
	gtkListItemFactoryArtists = gtk_signal_list_item_factory_new();
	columnViewArts = gtk_column_view_column_new("\t\t\t\t\t\t\t\t\t   Artist",
													gtkListItemFactoryArtists);
	gtk_column_view_column_set_expand(GTK_COLUMN_VIEW_COLUMN(columnViewArts), FALSE);
	gtk_column_view_column_set_fixed_width(GTK_COLUMN_VIEW_COLUMN(columnViewArts), 500);
	g_signal_connect(gtkListItemFactoryArtists, "setup", G_CALLBACK(setup_listitem_nums), NULL);
	g_signal_connect(gtkListItemFactoryArtists, "bind", G_CALLBACK(bind_listitem_arts), NULL);
	
	GtkListItemFactory *gtkListItemFactoryTitles;
	gtkListItemFactoryTitles = gtk_signal_list_item_factory_new();
	columnViewTitles = gtk_column_view_column_new("\t\t\t\t\t\t\t\t\t      Title",
															gtkListItemFactoryTitles);
	gtk_column_view_column_set_expand(GTK_COLUMN_VIEW_COLUMN(columnViewTitles), TRUE);
	g_signal_connect(gtkListItemFactoryTitles, "setup", G_CALLBACK(setup_listitem_nums), NULL);
	g_signal_connect(gtkListItemFactoryTitles, "bind", G_CALLBACK(bind_listitem_ttls), NULL);
	
	GtkListItemFactory *gtkListItemFactoryAlbums;
	gtkListItemFactoryAlbums = gtk_signal_list_item_factory_new();
	columnViewAlbums = gtk_column_view_column_new("\t\t\t\t\t\t\t      Album",
													gtkListItemFactoryAlbums);
	gtk_column_view_column_set_expand(GTK_COLUMN_VIEW_COLUMN(columnViewAlbums), FALSE);
	gtk_column_view_column_set_fixed_width(GTK_COLUMN_VIEW_COLUMN(columnViewAlbums), 433);
	g_signal_connect(gtkListItemFactoryAlbums, "setup", G_CALLBACK(setup_listitem_nums), NULL);
	g_signal_connect(gtkListItemFactoryAlbums, "bind", G_CALLBACK(bind_listitem_albums), NULL);
	
	GtkListItemFactory *gtkListItemFactoryDuration;
	gtkListItemFactoryDuration = gtk_signal_list_item_factory_new();
	columnViewDurations = gtk_column_view_column_new("     Duration", gtkListItemFactoryDuration);
	gtk_column_view_column_set_fixed_width(GTK_COLUMN_VIEW_COLUMN(columnViewDurations), 100);
	gtk_column_view_column_set_expand(GTK_COLUMN_VIEW_COLUMN(columnViewDurations), FALSE);
	g_signal_connect(gtkListItemFactoryDuration, "setup", G_CALLBACK(setup_listitem_nums), NULL);
	g_signal_connect(gtkListItemFactoryDuration, "bind", G_CALLBACK(bind_listitem_duration), NULL);

	// the playlist window
	browserList = gtk_column_view_new(selectionModel);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(browserList), columnViewNums);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(browserList), columnViewArts);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(browserList), columnViewTitles);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(browserList), columnViewAlbums);
	gtk_column_view_append_column(GTK_COLUMN_VIEW(browserList), columnViewDurations);
	gtk_column_view_set_show_column_separators(GTK_COLUMN_VIEW(browserList), TRUE);
	gtk_column_view_set_show_row_separators(GTK_COLUMN_VIEW(browserList), TRUE);
	gtk_widget_set_hexpand(browserList, FALSE);
	gtk_widget_set_vexpand(browserList, FALSE);
	gtk_widget_set_hexpand_set(browserList, FALSE);
	gtk_widget_set_vexpand_set(browserList, FALSE);

	g_signal_connect_swapped(browserList, "activate", G_CALLBACK(on_track_activate), NULL);

	// tracks list sctolling window
	trackListScrolledWindow = gtk_scrolled_window_new();
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(trackListScrolledWindow), browserList);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(trackListScrolledWindow), 575);

	// playback time scale adjustment
	adjScale = gtk_adjustment_new(0, 0, 100, 1, 10, 1);
	g_signal_connect_swapped(adjScale, "value-changed", G_CALLBACK(value_changed), NULL);
	// playback time scale
	GtkWidget *scale;
	scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, adjScale);
	gtk_scale_set_digits(GTK_SCALE(scale), 1);
	gtk_widget_set_vexpand(scale, FALSE);
	gtk_widget_set_hexpand(scale, FALSE);
	gtk_widget_set_hexpand_set(scale, FALSE);
	gtk_widget_set_vexpand_set(scale, FALSE);
	gtk_widget_set_valign(scale, GTK_ALIGN_CENTER);

	// frame playlists box
	GtkWidget *scale_frame;
	scale_frame = gtk_frame_new(NULL);
	gtk_widget_set_hexpand(scale_frame, FALSE);
	gtk_widget_set_vexpand(scale_frame, FALSE);
	gtk_widget_set_hexpand_set(scale_frame, FALSE);
	gtk_widget_set_vexpand_set(scale_frame, FALSE);
	gtk_frame_set_child(GTK_FRAME(scale_frame), scale);
	gtk_widget_set_margin_top(scale_frame, 7);
	gtk_widget_set_valign(scale_frame, GTK_ALIGN_END);

	// Playlist 1 | Playlist 2 | Playlist 3 | playlist
	GtkWidget *lab1_lab2_lab3_playlist_box;
	lab1_lab2_lab3_playlist_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_prepend(GTK_BOX(lab1_lab2_lab3_playlist_box), lab1_lab2_lab3_box);
	gtk_box_append(GTK_BOX(lab1_lab2_lab3_playlist_box), trackListScrolledWindow);
	gtk_box_append(GTK_BOX(lab1_lab2_lab3_playlist_box), scale_frame);
	gtk_widget_set_hexpand(lab1_lab2_lab3_playlist_box, FALSE);
	gtk_widget_set_vexpand(lab1_lab2_lab3_playlist_box, FALSE);
	gtk_widget_set_hexpand_set(lab1_lab2_lab3_playlist_box, FALSE);
	gtk_widget_set_vexpand_set(lab1_lab2_lab3_playlist_box, FALSE);

	// cov_hpbox_browscroll_pop | lab1_lab2_lab3_playlist_box both boxes into paned
	GtkWidget *left_right_boxes;
	left_right_boxes = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_start_child(GTK_PANED(left_right_boxes), cov_hpbox_browscroll_pop);
	gtk_paned_set_end_child(GTK_PANED(left_right_boxes), lab1_lab2_lab3_playlist_box);
	gtk_paned_set_resize_start_child(GTK_PANED(left_right_boxes), FALSE);
	gtk_paned_set_resize_end_child(GTK_PANED(left_right_boxes), TRUE);
	gtk_paned_set_shrink_start_child(GTK_PANED(left_right_boxes), FALSE);
	gtk_paned_set_shrink_end_child(GTK_PANED(left_right_boxes), FALSE);
	gtk_widget_set_hexpand(left_right_boxes, FALSE);
	gtk_widget_set_vexpand(left_right_boxes, FALSE);
	gtk_widget_set_hexpand_set(left_right_boxes, FALSE);
	gtk_widget_set_vexpand_set(left_right_boxes, FALSE);
	//gtk_paned_set_position(GTK_PANED(left_right_boxes), 303);
	// *********************************************************************************************
	// ***************************************  buttons ************************************
	// buttons close
	GtkWidget *close;
	close = gtk_button_new_with_label("Close");
	//gtk_widget_set_vexpand(close, FALSE);
	gtk_widget_set_halign(close, GTK_ALIGN_START);
	//gtk_widget_set_valign(close, GTK_ALIGN_START);
	g_signal_connect_swapped(close, "clicked", G_CALLBACK(gtk_window_close), dmelodyWindow);

	// buttons settings
	GtkWidget *settings;
	settings = gtk_button_new_from_icon_name("settings");
	gtk_widget_set_halign(settings, GTK_ALIGN_START);
	gtk_widget_set_vexpand(settings, FALSE);
	gtk_widget_set_valign(settings, GTK_ALIGN_START);
	g_signal_connect_swapped(settings, "clicked", G_CALLBACK(dmelody_settings), NULL);

	// buttons show constrols popup
	showControls = gtk_button_new_from_icon_name("media-eject");
	gtk_widget_set_halign(showControls, GTK_ALIGN_START);
	gtk_widget_set_vexpand(showControls, FALSE);
	gtk_widget_set_valign(showControls, GTK_ALIGN_START);
	g_signal_connect_swapped(showControls, "clicked", G_CALLBACK(open_control_bar), NULL);

	// previous track
	GtkWidget *previous;
	previous = gtk_button_new_from_icon_name("go-previous");
	gtk_widget_set_halign(previous, GTK_ALIGN_CENTER);
	gtk_widget_set_vexpand(previous, FALSE);
	gtk_widget_set_valign(previous, GTK_ALIGN_START);
	g_signal_connect_swapped(previous, "clicked", G_CALLBACK(previous_trackF), NULL);

	// play/pause
	playPause = gtk_button_new_from_icon_name("media-playback-start-symbolic");
	gtk_widget_set_halign(playPause, GTK_ALIGN_CENTER);
	gtk_widget_set_vexpand(playPause, FALSE);
	gtk_widget_set_valign(playPause, GTK_ALIGN_START);
	gtk_widget_set_size_request(playPause, 30, 30);
	g_signal_connect_swapped(playPause, "clicked", G_CALLBACK(play_pauseF), play_pause);

	// next track
	GtkWidget *next;
	next = gtk_button_new_from_icon_name("go-next");
	gtk_widget_set_halign(next, GTK_ALIGN_CENTER);
	gtk_widget_set_vexpand(next, FALSE);
	gtk_widget_set_valign(next, GTK_ALIGN_START);
	g_signal_connect_swapped(next, "clicked", G_CALLBACK(next_trackF), NULL);

	///////////////////////////////// repeat currect track
	// repeat label
	GtkWidget *repeatTrackLabel;
	repeatTrackLabel = gtk_label_new("Repeat current track");
	gtk_widget_set_halign(repeatTrackLabel, GTK_ALIGN_END);
	// switch repeat
	repeatTrack = gtk_switch_new();

	if (isPlaying) {
		if (!gtk_widget_is_sensitive(repeatTrack)) {
			gtk_widget_set_sensitive(repeatTrack, TRUE);
			gtk_switch_set_state(GTK_SWITCH(repeatTrack),
									get_int_value_from_conf(pathToConfigDmelody, "repeat"));
		}
	}
	else {
		gtk_widget_set_sensitive(repeatTrack, FALSE);
	}

	gtk_widget_set_halign(repeatTrack, GTK_ALIGN_END);
	//gtk_widget_set_valign(repeatTrack, GTK_ALIGN_START);
	watchRepeatSwitch = gtk_expression_watch(expressionStateSwitch, repeatTrack, notifyRepeatSw,
																				repeatTrack, NULL);

	// ___________________buttons left box__________________________
	GtkWidget *buttonsLeftBox;
	buttonsLeftBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
	gtk_box_prepend(GTK_BOX(buttonsLeftBox), close);
	gtk_box_append(GTK_BOX(buttonsLeftBox), settings);
	gtk_box_append(GTK_BOX(buttonsLeftBox), showControls);
	gtk_widget_set_vexpand(buttonsLeftBox, FALSE);
	gtk_widget_set_hexpand(buttonsLeftBox, FALSE);
	gtk_widget_set_vexpand(buttonsLeftBox, FALSE);
	gtk_widget_set_hexpand_set(buttonsLeftBox, FALSE);
	gtk_widget_set_vexpand_set(buttonsLeftBox, FALSE);
	gtk_widget_set_valign(buttonsLeftBox, GTK_ALIGN_START);
	gtk_widget_set_halign(buttonsLeftBox, GTK_ALIGN_START);
	gtk_widget_set_valign(buttonsLeftBox, GTK_ALIGN_END);

	// ___________________buttons center box__________________________
	GtkWidget *buttonsCenterBox;
	buttonsCenterBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 30);
	gtk_box_prepend(GTK_BOX(buttonsCenterBox), previous);
	gtk_box_append(GTK_BOX(buttonsCenterBox), playPause);
	gtk_box_append(GTK_BOX(buttonsCenterBox), next);
	gtk_widget_set_margin_start(buttonsCenterBox, 20);
	gtk_widget_set_vexpand(buttonsCenterBox, FALSE);
	gtk_widget_set_hexpand(buttonsCenterBox, FALSE);
	gtk_widget_set_vexpand(buttonsCenterBox, FALSE);
	gtk_widget_set_hexpand_set(buttonsCenterBox, FALSE);
	gtk_widget_set_vexpand_set(buttonsCenterBox, FALSE);
	gtk_widget_set_halign(buttonsCenterBox, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(buttonsCenterBox, GTK_ALIGN_START);

	// ___________________buttons right box__________________________
	GtkWidget *buttonsRightBox;
	buttonsRightBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 7);
	gtk_box_prepend(GTK_BOX(buttonsRightBox), repeatTrackLabel);
	gtk_box_append(GTK_BOX(buttonsRightBox), repeatTrack);
	gtk_widget_set_vexpand(buttonsRightBox, FALSE);
	//gtk_widget_set_hexpand(buttonsRightBox, FALSE);
	gtk_widget_set_vexpand(buttonsRightBox, FALSE);
	//gtk_widget_set_hexpand_set(buttonsRightBox, FALSE);
	gtk_widget_set_vexpand_set(buttonsRightBox, FALSE);
	gtk_widget_set_valign(buttonsRightBox, GTK_ALIGN_START);
	gtk_widget_set_halign(buttonsRightBox, GTK_ALIGN_END);

	// _______________________ button box_____________________________
	GtkWidget *buttonBox;
	buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(buttonBox), TRUE);
	gtk_box_prepend(GTK_BOX(buttonBox), buttonsLeftBox);
	gtk_box_append(GTK_BOX(buttonBox), buttonsCenterBox);
	gtk_box_append(GTK_BOX(buttonBox), buttonsRightBox);
	gtk_widget_set_margin_top(buttonBox, 10);
	gtk_widget_set_margin_bottom(buttonBox, 10);
	gtk_widget_set_vexpand(buttonBox, FALSE);
	gtk_widget_set_hexpand(buttonBox, FALSE);
	gtk_widget_set_vexpand(buttonBox, FALSE);
	gtk_widget_set_hexpand_set(buttonBox, FALSE);
	gtk_widget_set_vexpand_set(buttonBox, FALSE);
	gtk_widget_set_valign(buttonBox, GTK_ALIGN_START);

	// ___________________all the boxes__________________________
	GtkWidget *allBox;
	allBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
	gtk_box_prepend(GTK_BOX(allBox), left_right_boxes);
	gtk_box_append(GTK_BOX(allBox), buttonBox);
	gtk_widget_set_margin_start(allBox, 10);
	gtk_widget_set_margin_end(allBox, 10);
	gtk_widget_set_hexpand(allBox, FALSE);
	gtk_widget_set_vexpand(allBox, FALSE);
	gtk_widget_set_hexpand_set(allBox, FALSE);
	gtk_widget_set_vexpand_set(allBox, FALSE);
	gtk_widget_set_valign(allBox, GTK_ALIGN_FILL);
	//gtk_box_set_homogeneous(GTK_BOX(allBox), TRUE);

	GtkWidget *all_windows;
	all_windows = gtk_scrolled_window_new();
	//gtk_widget_set_valign(all_windows, GTK_ALIGN_CENTER);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(all_windows), allBox);

	// height
  	GValue valueHeight = G_VALUE_INIT;
	gtk_expression_watch_evaluate(watchHeight, &valueHeight);

	if (G_VALUE_HOLDS_INT(&valueHeight)) {
		gint heightValue = g_value_get_int(&valueHeight);
		gtk_widget_set_size_request(browserScrolledWindow, 0, heightValue - 460);
		gtk_widget_set_size_request(trackListScrolledWindow, 0, heightValue - 140);
	}

	// free resources
	g_object_unref(menu);
	g_object_unref(item1);
	g_object_unref(item2);
	g_object_unref(item3);
	g_object_unref(item4);
	g_object_unref(item5);
	g_object_unref(item6);
	g_object_unref(item7);
	g_object_unref(item8);
	g_value_unset(&valueHeight);

	// freing resources
	g_object_unref(layoutPlaylists);
	pango_attr_list_unref(attr_list_Playlists);

	gtk_window_set_child(GTK_WINDOW(dmelodyWindow), all_windows);
}

/// if arguments provided but application is not running
static int no_running_msg(GApplication *application) {
	(void)application;
	printf("DMelody not currently running!\n");
	return 1;
}

int main(int argc, char *argv[]) {
	// create initial config
	create_configs();

	gint status;

	GApplicationCommandLine *cmdline;
	app = gtk_application_new("com.github.DiogenesN.dmelody", G_APPLICATION_HANDLES_COMMAND_LINE);
	g_action_map_add_action_entries(G_ACTION_MAP(app), app_actions, G_N_ELEMENTS(app_actions), app);

	// makes sure to run arguments only if application is running
	if (argv[1] == NULL) {
		g_signal_connect(app, "activate", G_CALLBACK(activate), &cmdline);
		g_signal_connect(app, "startup", G_CALLBACK(startup), NULL);
		g_signal_connect(app, "command-line", G_CALLBACK(activate), &cmdline);
	}
	else {
		g_signal_connect(app, "command-line", G_CALLBACK(no_running_msg), NULL);
	}

	status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);
	return status;
}
