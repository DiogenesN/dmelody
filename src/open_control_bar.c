/* opens a bar with controls:
	* prev | play/pause | next   [ hide | toggle minimize/unminimize player | close player ]
 */

#include <glib.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include "externvars.h"
#include "getvaluefromconf.h"

GtkWidget *windowR;

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
static void hide_controls(GtkWidget *window) {
	toggleControlBar = 0;
	gtk_popover_popdown(GTK_POPOVER(APPNAME));
	gtk_window_close(GTK_WINDOW(window));
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
static void close_main_window(GtkWidget *window) {
	gtk_popover_popdown(GTK_POPOVER(APPNAME));
	gtk_window_close(GTK_WINDOW(window));
	gtk_window_close(GTK_WINDOW(dmelodyWindow));
}

/// opening the control bar
static void open_control_bar() {
	// toggle control bar
	if (toggleControlBar == 0) {
		toggleControlBar = 1;

		gint width	= get_int_value_from_conf(pathToConfigDmelody, "controlWidth");
		gint height	= get_int_value_from_conf(pathToConfigDmelody, "controlHeight");
		gint posx	= get_int_value_from_conf(pathToConfigDmelody, "controlX");
		gint posy	= get_int_value_from_conf(pathToConfigDmelody, "controlY");

		windowR = gtk_window_new();

		previous_track = gtk_button_new_from_icon_name("go-previous");
		gtk_widget_set_halign(previous_track, GTK_ALIGN_START);
		g_signal_connect_swapped(previous_track, "clicked", G_CALLBACK(previous_trackF), NULL);

		play_pause = gtk_button_new_from_icon_name("media-playback-start-symbolic");
		gtk_widget_set_halign(play_pause, GTK_ALIGN_START);
		g_signal_connect_swapped(play_pause, "clicked", G_CALLBACK(play_pauseF), play_pause);
	
		if (get_int_value_from_conf(pathToConfigDmelody, "autoplay") == 1) {
			isPlaying = TRUE;
			togglePlayPause = FALSE;
			if (GTK_IS_BUTTON(play_pause)) {
				gtk_button_set_icon_name(GTK_BUTTON(play_pause), "media-playback-pause-symbolic");
			}
			if (GTK_IS_BUTTON(playPause)) {
				gtk_button_set_icon_name(GTK_BUTTON(playPause), "media-playback-pause-symbolic");
			}
		}
		else if (isPlaying && !togglePlayPause) {
			if (GTK_IS_BUTTON(play_pause)) {
				gtk_button_set_icon_name(GTK_BUTTON(play_pause), "media-playback-pause-symbolic");
			}
			if (GTK_IS_BUTTON(playPause)) {
				gtk_button_set_icon_name(GTK_BUTTON(playPause), "media-playback-pause-symbolic");
			}
		}
		else if (isPlaying && togglePlayPause) {
			if (GTK_IS_BUTTON(play_pause)) {
				gtk_button_set_icon_name(GTK_BUTTON(play_pause), "media-playback-start-symbolic");
			}
			if (GTK_IS_BUTTON(playPause)) {
				gtk_button_set_icon_name(GTK_BUTTON(playPause), "media-playback-start-symbolic");
			}
		}

		next_track = gtk_button_new_from_icon_name("go-next");
		gtk_widget_set_halign(next_track, GTK_ALIGN_START);
		g_signal_connect_swapped(next_track, "clicked", G_CALLBACK(next_trackF), NULL);

		GtkWidget *label;
		label = gtk_label_new(NULL);
		gtk_widget_set_halign(label, GTK_ALIGN_CENTER);

		GtkWidget *hideControls;
		hideControls = gtk_button_new_from_icon_name("window-minimize-symbolic");
		gtk_widget_set_halign(hideControls, GTK_ALIGN_END);
		g_signal_connect_swapped(hideControls, "clicked", G_CALLBACK(hide_controls), windowR);

		GtkWidget *showPlayer;
		showPlayer = gtk_button_new_from_icon_name("sidebar-show-symbolic");
		gtk_widget_set_halign(showPlayer, GTK_ALIGN_END);
		g_signal_connect_swapped(showPlayer, "clicked", G_CALLBACK(show_main_window), windowR);

		GtkWidget *closeButton;
		closeButton = gtk_button_new_from_icon_name("window-close-symbolic");
		gtk_widget_set_halign(closeButton, GTK_ALIGN_END);
		g_signal_connect_swapped(closeButton, "clicked", G_CALLBACK(close_main_window), windowR);

		GtkWidget *popBox;
		popBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
		gtk_box_set_homogeneous(GTK_BOX(popBox), TRUE);
		gtk_box_prepend(GTK_BOX(popBox), previous_track);
		gtk_box_append(GTK_BOX(popBox), play_pause);
		gtk_box_append(GTK_BOX(popBox), next_track);
		gtk_box_append(GTK_BOX(popBox), label);
		gtk_box_append(GTK_BOX(popBox), hideControls);
		gtk_box_append(GTK_BOX(popBox), showPlayer);
		gtk_box_append(GTK_BOX(popBox), closeButton);

		APPNAME = gtk_popover_new();
		gtk_widget_set_size_request(APPNAME, width, height);
		gtk_popover_set_child(GTK_POPOVER(APPNAME), popBox);
		gtk_popover_set_autohide(GTK_POPOVER(APPNAME), FALSE);
		gtk_popover_set_has_arrow(GTK_POPOVER(APPNAME), FALSE);
		gtk_popover_set_offset(GTK_POPOVER(APPNAME), posx, posy);

		gtk_window_set_child(GTK_WINDOW(windowR), APPNAME);
		gtk_popover_popup(GTK_POPOVER(APPNAME));
	}
	else {
		toggleControlBar = 0;
		hide_controls(windowR);
	}
}
