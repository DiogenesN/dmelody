#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "externvars.h"
#include "replacenthline.h"
#include "getvaluefromconf.h"

// action on third tab clicked
void on_playlist3_l_click() {
	tabClicked = get_int_value_from_conf(pathToConfigDmelody, "lastTabClicked");
	if (tabClicked == 1) {
		nrOfElements = get_number_of_lines(pathToPlaylist1);
	}
	else if (tabClicked == 2) {
		nrOfElements = get_number_of_lines(pathToPlaylist2);
	}
	else {
		nrOfElements = get_number_of_lines(pathToPlaylist3);
	}

	// if playlist is empty and nothing added to the playlist then return
	if (nrOfElements < 1 && get_number_of_lines(pathToPlaylist3) < 1) {
		return;
	}
	if (get_number_of_lines(pathToPlaylist3) == 0) {
		return;
	}

	printf("prev tab: %d\n", tabClicked);

	gchar *lastTabClicked = "lastTabClicked=3";
	replacenthline(pathToConfigDmelody, 11, lastTabClicked);

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
	
	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewNums);
	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewArts);
	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewTitles);
	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewAlbums);
	gtk_column_view_remove_column(GTK_COLUMN_VIEW(browserList), columnViewDurations);

	gtk_widget_set_visible(browserList, FALSE);

	populate_allNumbers(pathToConfigDmelodyDirNumbersFile3);
	populate_allArtists(pathToConfigDmelodyDirArtistsFile3);
	populate_allTitles(pathToConfigDmelodyDirTitlesFile3);
	populate_allAlbums(pathToConfigDmelodyDirAlbumsFile3);
	populate_allDurations(pathToConfigDmelodyDirDurationsFIle3);

	gtk_string_list_splice(numbers, 0, nrOfElements, (const char * const *)allNumbers);
	gtk_string_list_splice(artists, 0, nrOfElements, (const char * const *)allArtists);
	gtk_string_list_splice(titles, 0, nrOfElements, (const char * const *)allTitles);
	gtk_string_list_splice(albums, 0, nrOfElements, (const char * const *)allAlbums);
	gtk_string_list_splice(durations, 0, nrOfElements, (const char * const *)allDurations);

	ListModelNumbers = G_LIST_MODEL(numbers);
	gtk_column_view_set_model(GTK_COLUMN_VIEW(browserList), selectionModel);
	gtk_widget_set_visible(browserList, TRUE);

	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 0, columnViewNums);
	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 1, columnViewArts);
	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 2, columnViewTitles);
	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 3, columnViewAlbums);
	gtk_column_view_insert_column(GTK_COLUMN_VIEW(browserList), 4, columnViewDurations);

	// setting cover imaget from conf files
	gtk_image_set_from_file(GTK_IMAGE(cover), get_char_value_from_conf(pathToConfigDmelody,
																			"playlist3cover"));
}
