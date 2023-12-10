/* opens a bar with controls:
	* prev | play/pause | next   [ hide | toggle minimize/unminimize player | close player ]
 */

#include <glib.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include "externvars.h"
#include "getvaluefromconf.h"

static gint toggleControlBar = 0;

/// previous track
void previous_trackF() {
	// checking if any tracks have been added to playists at all
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

	if (isPlaying && nrOfLines >= 1) {
		send_mpv_cmd("/tmp/mpvsocket", "playlist-prev\n");
	}
}

/// play / pause
void play_pauseF(GtkWidget *button) {
	// activating repeat switch
	if (!gtk_widget_is_sensitive(repeatTrack)) {
		gtk_widget_set_sensitive(repeatTrack, TRUE);
		gtk_switch_set_state(GTK_SWITCH(repeatTrack),
									get_int_value_from_conf(pathToConfigDmelody, "repeat"));
	}
	// checking if any tracks have been added to playists at all
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

	// if newly started and is not playing then start playing
	if (!isPlaying && nrOfLines >= 1) {
		printf("play 1\n");
		togglePlayPause = FALSE;
		g_signal_emit_by_name(browserList, "activate");
		if (GTK_IS_BUTTON(button)) {
			gtk_button_set_icon_name(GTK_BUTTON(button), "media-playback-pause-symbolic");
		}
		if (GTK_IS_BUTTON(playPause)) {
			gtk_button_set_icon_name(GTK_BUTTON(playPause), "media-playback-pause-symbolic");
		}
		return;
	}

	// if is playing then pause, if pause then play and set icons accordingly
	if (isPlaying && togglePlayPause) {
		printf("play 2\n");
		togglePlayPause = FALSE;
		if (GTK_IS_BUTTON(button)) {
			gtk_button_set_icon_name(GTK_BUTTON(button), "media-playback-pause-symbolic");
		}
		if (GTK_IS_BUTTON(playPause)) {
			gtk_button_set_icon_name(GTK_BUTTON(playPause), "media-playback-pause-symbolic");
		}
		// toggle play/pause command
		send_mpv_cmd("/tmp/mpvsocket", "cycle pause\n");
		return;
	}
	else if (isPlaying && !togglePlayPause) {
		printf("play 3\n");
		togglePlayPause = TRUE;
		if (GTK_IS_BUTTON(button)) {
			gtk_button_set_icon_name(GTK_BUTTON(button), "media-playback-start-symbolic");
		}
		if (GTK_IS_BUTTON(playPause)) {
			gtk_button_set_icon_name(GTK_BUTTON(playPause), "media-playback-start-symbolic");
		}
		// toggle play/pause command
		send_mpv_cmd("/tmp/mpvsocket", "cycle pause\n");
		return;
	}
}

/// next track
void next_trackF() {
	// checking if any tracks have been added to playists at all
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

	if (isPlaying && nrOfLines >= 1) {
		send_mpv_cmd("/tmp/mpvsocket", "playlist-next\n");
	}
}

/// hide controls bar
static void hide_controls() {
	toggleControlBar = 0;
	gtk_popover_popdown(GTK_POPOVER(APPNAME));
}

/// toggle unminimize/minimize back main player window without closing controls
static void show_main_window() {
	// check if main player window is active, if active then minimize and unminimize otherwise
	if (gtk_window_is_active(GTK_WINDOW(dmelodyWindow))) {
		gtk_window_minimize(GTK_WINDOW(dmelodyWindow));
	}
	else {
		gtk_window_present(GTK_WINDOW(dmelodyWindow));
	}
}

/// closes the application completely
static void close_main_window() {
	gtk_popover_popdown(GTK_POPOVER(APPNAME));
	gtk_window_close(GTK_WINDOW(dmelodyWindow));
}

/// opening the control bar
static void open_control_bar() {
	// toggle control bar
	if (toggleControlBar == 0) {
		toggleControlBar = 1;

		gtk_popover_popup(GTK_POPOVER(APPNAME));
		gtk_popover_popdown(GTK_POPOVER(APPNAME));
		gtk_popover_popup(GTK_POPOVER(APPNAME));
	}
	else {
		toggleControlBar = 0;
		hide_controls();
	}
}
