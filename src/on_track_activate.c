/// double click on track starts playing
#include <glib.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include "externvars.h"
#include "sendcmdtompv.h"
#include "getintvaluefromjson.h"

gint valuePos;
gint upperAdj;
gint total_hours;
gint total_minutes;
gint total_seconds;
gint elapsed_hours;
gint elapsed_minutes;
gint elapsed_seconds;
gint counterPos = 1;
const char *key = "data";
const char *property_tag = "get_property";

// seek by the number of seconds in the currently playing track
void value_changed() {
	// if track finished and a new one started
	if (valuePos == 0) {
		counterPos = 0;
		// checks if repeat switch is on and sets the playback on repeat mode
		if (get_int_value_from_conf(pathToConfigDmelody, "repeat") == 1) {
			g_print("Repeat is on\n");
			send_mpv_cmd("/tmp/mpvsocket", "cycle-values loop-file \"inf\" \"yes\"\n");
		}
	}

	// seek to current position
	if (valuePos != counterPos) {
		gint seek_seconds = (gint)gtk_adjustment_get_value(adjScale);
		const char *seekCMD[] = {"seek ", "%d", " absolute"};
		char full_seekCMD[256];
		snprintf(full_seekCMD, sizeof(full_seekCMD), "%s%d%s\n", seekCMD[0], seek_seconds, seekCMD[2]);
		send_mpv_cmd("/tmp/mpvsocket", full_seekCMD);
		counterPos = seek_seconds;
		valuePos = seek_seconds;
	}

	counterPos = counterPos + 1;
}

/// adjusting slider properties and setting selected track to the currently playing trach
gboolean playing_mpv(gpointer data) {
	(void)data;

	if (isWindowOpened) {
		valuePos = get_int_value_from_json(property_tag, "time-pos", key);
		upperAdj = valuePos + get_int_value_from_json(property_tag, "time-remaining", key);

		// set upper limit of the slider so the track duration matches slider length
		gtk_adjustment_set_upper(adjScale, upperAdj);

		// changes selection to the currently playying track
		gtk_single_selection_set_selected(singleSelectionNums,
										get_int_value_from_json(property_tag, "playlist-pos", key));

		// changes time slider position accordint to seconds passed in the track
		gtk_adjustment_set_value(adjScale, valuePos);

		int total_hours = upperAdj / 3600;
		int total_minutes = (upperAdj % 3600) / 60;
		int total_seconds = upperAdj % 60;

		int elapsed_hours = valuePos / 3600;
		int elapsed_minutes = (valuePos % 3600) / 60;
		int elapsed_seconds = valuePos % 60;

		// Create the formatted string using g_strdup_printf
		gchar *formattedTime = g_strdup_printf("%02d:%02d:%02d / %02d:%02d:%02d",
																	elapsed_hours,
																	elapsed_minutes,
																	elapsed_seconds,
																	total_hours,
																	total_minutes,
																	total_seconds);

		gtk_label_set_label(GTK_LABEL(timeElapsed), formattedTime);
		g_free(formattedTime);
	}
	else {
		printf("Closing DMelody\n");
		g_spawn_close_pid(pid);
		return G_SOURCE_REMOVE | FALSE;
	}
	return TRUE;
}

/// activates and plays the track on double click
void on_track_activate() {
	counterPos = 1;
	isPlaying = TRUE;

	gtk_button_set_icon_name(GTK_BUTTON(playPause), "media-playback-pause-symbolic");

	if (togglePlayPause) {
		togglePlayPause = FALSE;
		if (GTK_IS_BUTTON(play_pause)) {
			gtk_button_set_icon_name(GTK_BUTTON(play_pause), "media-playback-pause-symbolic");
		}
	}

	// getting which tab is currently opened also this is the number of active playlist
	gboolean result;
	GError *error = NULL;

	// getting the path of the current playlist accorind to the active tab
	gchar pathToCurrentPlaylist[1024];
	tabClicked = get_int_value_from_conf(pathToConfigDmelody, "lastTabClicked");

	if (tabClicked == 1) {
		strcpy(pathToCurrentPlaylist, pathToPlaylist1);
	}
	else if (tabClicked == 2) {
		strcpy(pathToCurrentPlaylist, pathToPlaylist2);
	}
	else {
		strcpy(pathToCurrentPlaylist, pathToPlaylist3);
	}

	gchar path[100];
	snprintf(path, sizeof(path) + strlen(pathToCurrentPlaylist),
												"--playlist=%s", pathToCurrentPlaylist);

	gchar position[100];
	snprintf(position, sizeof(position), "--playlist-start=%d",
									gtk_single_selection_get_selected(singleSelectionNums));

	// getting the position of the clicked track to be used as argument in mpv --playlist-start=int
	gchar *mpvCMD[] = { "mpv", "--hwdec", "--no-video", "--really-quiet", "--loop-playlist",
					   "--input-ipc-server=/tmp/mpvsocket", path, position, NULL };

	// quitting mpv (if running) before playing another track
	if (pid != 0) {
		send_mpv_cmd("/tmp/mpvsocket", "quit\n");
	}

	// starts the actual playback
	result = g_spawn_async(
					NULL,
					mpvCMD,
					NULL,
					G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
					NULL,
					NULL,
					&pid,
					&error);
	
	if (!result) {
		g_print("Error: %s\n", error->message);
		g_error_free(error);
	}

	// runs loop to set selection to currently playing track
	g_timeout_add_seconds(1, playing_mpv, NULL);

	sleep(1);
	// unfreezing repeat switch button and setting the state
	gtk_widget_set_sensitive(repeatTrack, TRUE);
	gtk_switch_set_state(GTK_SWITCH(repeatTrack),
									get_int_value_from_conf(pathToConfigDmelody, "repeat"));
}
