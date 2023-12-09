#include <glib.h>
#include <stdio.h>
#include <string.h>
#include "externvars.h"
#include "outputtochar.h"
#include "protofunctions.h"
#include "replacenthline.h"
#include "getnumberoflines.h"
#include "getvaluefromconf.h"
#include "populateallstrings.h"
#include "charrarrfromstrliteral.h"
#include "getfilenamecontainingsubstr.h"
#include "getstrsfromsubstrswritetoconf.h"

#define LINENUMBERTOREPLACE 8
#define PLAYLISTLABEL		playlist1
#define NAMEOFADDFUNC		addtopl1_cb
#define PLAYLISTNAME		"playlist1name="
#define PATHTOPLAYLIST		pathToPlaylist1
#define NAMEOFADDWAITFUNC	waiting_populating_a1_files

//////////////////////// replace playlist 1 action ////////////////////////////////////////

gboolean NAMEOFADDWAITFUNC(gpointer data) {
	(void)data;
	////////////////////////////// EXTRACTING EXIF DAYA //////////////////////////////////
	g_print("\nExtracting exif data\n\n");

	//gint nrOfLines = get_number_of_lines(pathToPlaylist1);
	char *awkCMDS = " | awk '{line=\"\"; for (i=3; i<=NF; i++) line = line $i \" \"; print line}' | cut -c 2-";
	char *awkCMD = " | awk '{line=\"\"; for (i=3; i<=NF; i++) line = line $i \" \"; print line}'";
	int lineCount = 0;

	FILE *playlist = fopen(PATHTOPLAYLIST, "r");
	FILE *numbersF = fopen(pathToConfigDmelodyDirNumbersFile, "w");
	FILE *artistsF = fopen(pathToConfigDmelodyDirArtistsFile, "w");
	FILE *titlesF = fopen(pathToConfigDmelodyDirTitlesFile, "w");
	FILE *albumsF = fopen(pathToConfigDmelodyDirAlbumsFile, "w");
	FILE *durationsF = fopen(pathToConfigDmelodyDirDurationsFIle, "w");

	char strs_full_path_to_track[1024];

	// main loop
	while (fgets(strs_full_path_to_track, sizeof(strs_full_path_to_track), playlist) != NULL) {
		// removing new line character
		size_t length = strlen(strs_full_path_to_track);
		if (length > 0 && strs_full_path_to_track[length - 1] == '\n') {
			strs_full_path_to_track[length - 1] = '\0';
		}

		fprintf(numbersF, "%d\n", lineCount);

		// replcae artists in artists file from playlist exiftool
		char exifArtistsCMD[2048];
		snprintf(exifArtistsCMD, 2048, "%s \"%s\" %s", EXIFARTIST, strs_full_path_to_track, awkCMD);
		char ArtistsName[1024];
		strcpy(ArtistsName, output_to_char(exifArtistsCMD));
		if (strcmp(ArtistsName, "-") == 0 || strcmp(ArtistsName, "") == 0) {
			strcpy(ArtistsName, "Unknown Artist");
		}
		fprintf(artistsF, "%s\n", ArtistsName);

		// replcae titles in artists file from playlist exiftool
		char exifTitlesCMD[1024];
		snprintf(exifTitlesCMD, 2048, "%s \"%s\" %s", EXIFTITLE, strs_full_path_to_track, awkCMD);
		char TitleName[1024];
		strcpy(TitleName, output_to_char(exifTitlesCMD));
		if (strcmp(TitleName, "-") == 0 || strcmp(TitleName, "") == 0) {
			snprintf(exifTitlesCMD, 2048, "%s \"%s\" %s", EXIFFILENAME, strs_full_path_to_track, awkCMDS);
			strcpy(TitleName, output_to_char(exifTitlesCMD));
		}
		fprintf(titlesF, "%s\n", TitleName);

		// replcae albumn in artists file from playlist exiftool
		char exifAlbumsCMD[1024];
		snprintf(exifAlbumsCMD, 2048, "%s \"%s\" %s", EXIFALBUM, strs_full_path_to_track, awkCMD);
		char AlbumName[1024];
		strcpy(AlbumName, output_to_char(exifAlbumsCMD));
		if (strcmp(AlbumName, "-") == 0 || strcmp(AlbumName, "") == 0) {
			strcpy(AlbumName, "Unknown Album");
		}
		fprintf(albumsF, "%s\n", AlbumName);

		// replcae durations in artists file from playlist exiftool
		char exifDurationsCMD[1024];
		snprintf(exifDurationsCMD, 2048, "%s \"%s\" %s", EXIFDURATION, strs_full_path_to_track, awkCMD);
		char DurationName[1024];
		strcpy(DurationName, output_to_char(exifDurationsCMD));
		if (strcmp(DurationName, "-") == 0 || strcmp(DurationName, "") == 0) {
			strcpy(DurationName, "?.??.??");
		}
		fprintf(durationsF, "%s\n", DurationName);

		// increase line number counter
		lineCount = lineCount + 1;
	}

	fclose(durationsF);
	fclose(albumsF);
	fclose(titlesF);
	fclose(artistsF);
	fclose(numbersF);
	fclose(playlist);

	populate_allNumbers(pathToConfigDmelodyDirNumbersFile);
	populate_allArtists(pathToConfigDmelodyDirArtistsFile);
	populate_allTitles(pathToConfigDmelodyDirTitlesFile);
	populate_allAlbums(pathToConfigDmelodyDirAlbumsFile);
	populate_allDurations(pathToConfigDmelodyDirDurationsFIle);

	gtk_string_list_splice(numbers, 0, nrOfElements, (const char * const *)allNumbers);
	gtk_string_list_splice(artists, 0, nrOfElements, (const char * const *)allArtists);
	gtk_string_list_splice(titles, 0, nrOfElements, (const char * const *)allTitles);
	gtk_string_list_splice(albums, 0, nrOfElements, (const char * const *)allAlbums);
	gtk_string_list_splice(durations, 0, nrOfElements, (const char * const *)allDurations);

	gtk_popover_popdown(GTK_POPOVER(waitingPopup));

	ListModelNumbers = G_LIST_MODEL(numbers);
	gtk_column_view_set_model(GTK_COLUMN_VIEW(browserList), selectionModel);
	gtk_widget_set_visible(browserList, TRUE);

	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 0, columnViewNums);
	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 1, columnViewArts);
	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 2, columnViewTitles);
	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 3, columnViewAlbums);
	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 4, columnViewDurations);

	// set cover art and writing new path to corresponding line in conf
	gchar coverArtPathConfName[2048];
	strcpy(coverArtPathConfName, "playlist1cover=");
	strcat(coverArtPathConfName, coverArtFullPath);
	replacenthline(pathToConfigDmelody, 12, coverArtPathConfName);

	gtk_image_set_from_file(GTK_IMAGE(cover), coverArtFullPath);

	return G_SOURCE_REMOVE | FALSE;
}

static void NAMEOFADDFUNC() {
	nrOfElements = g_list_model_get_n_items(ListModelNumbers);

	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewNums);
	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewArts);
	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewTitles);
	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewAlbums);
	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewDurations);

	gtk_widget_set_visible(browserList, FALSE);

	gtk_popover_popup(GTK_POPOVER(waitingPopup));

	g_print("Replace Playlist 1\n");
	const gchar *extStr = get_char_value_from_conf(pathToConfigDmelody, "formats");
	const gchar *extArr[64];
	char folderNameForPlaylistLabel[1024];
	char folderNameLineToReplace[2048];
	chararr_from_strliteral_tok(extStr, ", ", extArr, 64);

	addToPlaylistState = 1;

	GtkStringObject *myItem;
	myItem = gtk_single_selection_get_selected_item(singleSelectionDirs);
	const gchar *selectedItem = gtk_string_object_get_string(myItem);
	const gchar *musicPath = get_char_value_from_conf(pathToConfigDmelody, "musicpath");

	if (strlen(fullPathToSelDir) == 0) {
		strcpy((char *)fullPathToSelDir, musicPath);
		strcpy((char *)fullPathToSelDir, musicPath);
		strcat((char *)fullPathToSelDir, "/");
		strcat((char *)fullPathToSelDir, selectedItem);
	}
	else if (strlen(fullPathToSelDir) != 0 && openDirState == 0 && addToPlaylistState2 == 0) {
		strcpy((char *)fullPathToSelDir, musicPath);
		strcat((char *)fullPathToSelDir, "/");
		strcat((char *)fullPathToSelDir, selectedItem);
	}
	else if (strlen(fullPathToSelDir) != 0 && openDirState != 0 && addToPlaylistState2 == 0) {
		// removing 'Open directory option
		g_menu_remove(menu, 0);
		g_menu_prepend(menu, "Open directory", "none");
		addToPlaylistState2 = 1;
		strcat((char *)fullPathToSelDir, "/");
		strcat((char *)fullPathToSelDir, selectedItem);
	}
	else {
		printf("Modifying the path\n");
		// Find the last occurrence of '/'
		char *lastSlash = strrchr(fullPathToSelDir, '/');

		if (lastSlash != NULL) {
			// Null-terminate the string at the last slash
			*lastSlash = '\0';

			strcat((char *)fullPathToSelDir, "/");
			strcat((char *)fullPathToSelDir, selectedItem);
		}
	}

	g_print("Selected music path: %s\n", fullPathToSelDir);
	g_print("Playlist 1 path: %s\n", PATHTOPLAYLIST);

	// getting folder name to use as label for playlist name (after last slash)
	// Find the last occurrence of the slash
	char *lastSlash = strrchr(fullPathToSelDir, '/');
	// Check if the slash was found
	if (lastSlash != NULL) {
		// Calculate the length of the substring after the last slash
		size_t length = strlen(lastSlash + 1);

		// Ensure the length is within bounds
		if (length < sizeof(char) * 1024) {
			// Create a new char array

			// Copy the substring after the last slash into the new array
			strcpy(folderNameForPlaylistLabel, lastSlash + 1);
		}
		else {
			perror("Error: Substring length exceeds the size of the char array.\n");
		}
	}
	else {
		perror("Error: Slash not found in the original string.\n");
	}

	// writing changes to configs and set the playlist label to the current directory
	snprintf(folderNameLineToReplace, sizeof(folderNameLineToReplace), PLAYLISTNAME"%s\n",
																		folderNameForPlaylistLabel);
	replacenthline(pathToConfigDmelody, 8, folderNameLineToReplace);

	// defining the size of text in label
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(20 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoAttribute *weight_attr = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
	pango_attr_list_insert(attr_list, weight_attr);

	PangoLayout *layout = gtk_widget_create_pango_layout(PLAYLISTLABEL, NULL);
	pango_layout_set_attributes(layout, attr_list);
	gtk_label_set_attributes(GTK_LABEL(PLAYLISTLABEL), attr_list);
	gtk_label_set_attributes(GTK_LABEL(playlist2), NULL);
	gtk_label_set_attributes(GTK_LABEL(playlist3), NULL);
	gtk_label_set_label(GTK_LABEL(PLAYLISTLABEL), folderNameForPlaylistLabel);

	g_object_unref(layout);
	pango_attr_list_unref(attr_list);

	// getting coverArt image in the selected directory
	const char *coverArt = get_filename_containing_substring(fullPathToSelDir, "front.jpg");

	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, "front.png");
	}
	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, "Front.jpg");
	}
	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, "Front.png");
	}

	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, "Cover.jpg");
	}

	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, "Cover.png");
	}

	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, "cover.jpg");
	}

	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, "cover.png");
	}

	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, ".jpg");
	}

	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, ".jpeg");
	}

	if (coverArt == NULL) {
		coverArt = get_filename_containing_substring(fullPathToSelDir, ".png");
	}

	// if coverArt found then set the coverArt for playlist 1 tab
	if (coverArt != NULL) {
		printf("cover Art: %s\n", coverArt);
		// writing coverArt full path to config line
		gchar coverArtPathConfName[2048];
		snprintf(coverArtFullPath, sizeof(coverArtFullPath), "%s", coverArt);
		snprintf(coverArtPathConfName, sizeof(coverArtPathConfName), "playlist1cover=%s\n", coverArt);
		replacenthline(pathToConfigDmelody, 12, coverArtPathConfName);
	}
	else {
		printf("cover Art not found\n");
		strcpy(coverArtFullPath, defaultEmptyCover);
	}

	openDirState = 0;

	gchar *lastTabClicked = "lastTabClicked=1";
	replacenthline(pathToConfigDmelody, 11, lastTabClicked);

	// looking for audio files extensions in selected directory and writing found files.extension to config
	get_strs_from_substrs_write_to_conf(fullPathToSelDir, extArr, PATHTOPLAYLIST, "a");

	g_timeout_add_seconds(1, NAMEOFADDWAITFUNC, NULL);
}
