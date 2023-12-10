#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "externvars.h"
#include "replacenthline.h"
#include "getvaluefromconf.h"

GtkWidget *holdsExtensions;

GtkExpressionWatch *watchAutoSwitch;
GtkExpressionWatch *watchMiniSwitch;
GtkExpressionWatch *watchControlsSwitch;

int save_close(GtkWindow *window) {
	gint width = gtk_adjustment_get_value(adjustmentSizeWidth);
	gchar widthText[30];
	sprintf(widthText, "controlWidth=%d", width);
	replacenthline(pathToConfigDmelody, 16, (const char *)widthText);

	gint height = gtk_adjustment_get_value(adjustmentSizeHeight);
	gchar heightText[30];
	sprintf(heightText, "controlHeight=%d", height);
	replacenthline(pathToConfigDmelody, 17, (const char *)heightText);

	gint posx = gtk_adjustment_get_value(adjustmentPosx);
	gchar posxText[30];
	sprintf(posxText, "controlX=%d", posx);
	replacenthline(pathToConfigDmelody, 18, (const char *)posxText);

	gint posy = gtk_adjustment_get_value(adjustmentPosy);
	gchar posyText[30];
	sprintf(posyText, "controlY=%d", posy);
	replacenthline(pathToConfigDmelody, 19, (const char *)posyText);

	GtkEntryBuffer *buffentryFormats;
	buffentryFormats = gtk_entry_get_buffer(GTK_ENTRY(holdsExtensions));
	const char *entryFormatsText = gtk_entry_buffer_get_text(buffentryFormats);

	gchar *writeFormats = g_strdup_printf("formats=%s", entryFormatsText);
	replacenthline(pathToConfigDmelody, 6, writeFormats);
	g_free(writeFormats);

	if (GTK_IS_EXPRESSION(watchMiniSwitch)) {
		gtk_expression_watch_unwatch(watchAutoSwitch);
	}

	if (GTK_IS_EXPRESSION(watchMiniSwitch)) {
		gtk_expression_watch_unwatch(watchMiniSwitch);
	}

	if (GTK_IS_EXPRESSION(watchControlsSwitch)) {
		gtk_expression_watch_unwatch(watchControlsSwitch);
	}

	gtk_window_close(GTK_WINDOW(window));
	return false;
}

static void notifyAutoSw(gpointer data) {
	gchar *writeAutoplay = g_strdup_printf("autoplay=%d", gtk_switch_get_state(GTK_SWITCH(data)));
	replacenthline(pathToConfigDmelody, 3, writeAutoplay);
	g_free(writeAutoplay);
}

static void notifyMiniSw(gpointer data) {
	gchar *writeMinim = g_strdup_printf("startminimized=%d", gtk_switch_get_state(GTK_SWITCH(data)));
	replacenthline(pathToConfigDmelody, 15, writeMinim);
	g_free(writeMinim);
}

static void notifyControlsSw(gpointer data) {
	gchar *writeControls = g_strdup_printf("controlbar=%d", gtk_switch_get_state(GTK_SWITCH(data)));
	replacenthline(pathToConfigDmelody, 5, writeControls);
	g_free(writeControls);
}

void dmelody_settings() {
	GtkWidget *windowSettings;
	windowSettings = gtk_window_new();
	gtk_window_set_modal(GTK_WINDOW(windowSettings), TRUE);
	gtk_window_set_decorated(GTK_WINDOW(windowSettings), FALSE);
	gtk_window_set_default_size(GTK_WINDOW(windowSettings), 700, 0);
	gtk_window_set_transient_for(GTK_WINDOW(windowSettings), GTK_WINDOW(dmelodyWindow));

	///////////////////////////////// automatically start playback on startup
	GtkWidget *autoplay;
	autoplay = gtk_switch_new();
	gtk_switch_set_state(GTK_SWITCH(autoplay),
									get_int_value_from_conf(pathToConfigDmelody, "autoplay"));
	gtk_widget_set_halign(autoplay, GTK_ALIGN_START);
	gtk_widget_set_valign(autoplay, GTK_ALIGN_START);
	watchAutoSwitch = gtk_expression_watch(expressionStateSwitch, autoplay, notifyAutoSw, autoplay,
																								NULL);
	// autoplay label
	GtkWidget *autoplayLabel;
	autoplayLabel = gtk_label_new("Automatically start playback");
	gtk_widget_set_halign(autoplayLabel, GTK_ALIGN_START);
	gtk_widget_set_valign(autoplayLabel, GTK_ALIGN_START);
	// autoplay and label box
	GtkWidget *autoplay_label_box;
	autoplay_label_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
	gtk_box_prepend(GTK_BOX(autoplay_label_box), autoplay);
	gtk_box_append(GTK_BOX(autoplay_label_box), autoplayLabel);

	///////////////////////////////// minimized automatically on startup
	GtkWidget *minimize;
	minimize = gtk_switch_new();
	gtk_switch_set_state(GTK_SWITCH(minimize),
									get_int_value_from_conf(pathToConfigDmelody, "startminimized"));
	gtk_widget_set_halign(minimize, GTK_ALIGN_START);
	gtk_widget_set_valign(minimize, GTK_ALIGN_START);
	watchMiniSwitch = gtk_expression_watch(expressionStateSwitch, minimize, notifyMiniSw, minimize,
																								NULL);
	// minimize label
	GtkWidget *minimizeLabel;
	minimizeLabel = gtk_label_new("Start minimized");
	gtk_widget_set_halign(minimizeLabel, GTK_ALIGN_START);
	gtk_widget_set_valign(minimizeLabel, GTK_ALIGN_START);
	// autoplay and label box
	GtkWidget *minimiz_label_box;
	minimiz_label_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
	gtk_box_prepend(GTK_BOX(minimiz_label_box), minimize);
	gtk_box_append(GTK_BOX(minimiz_label_box), minimizeLabel);

	///////////////////////////////// show control bar on startup
	GtkWidget *controlBar;
	controlBar = gtk_switch_new();
	gtk_switch_set_state(GTK_SWITCH(controlBar),
									get_int_value_from_conf(pathToConfigDmelody, "controlbar"));
	gtk_widget_set_halign(controlBar, GTK_ALIGN_START);
	gtk_widget_set_valign(controlBar, GTK_ALIGN_START);
	// control bar label
	GtkWidget *controlBarLabel;
	controlBarLabel = gtk_label_new("Show control bar on startup");
	gtk_widget_set_halign(controlBarLabel, GTK_ALIGN_START);
	gtk_widget_set_valign(controlBarLabel, GTK_ALIGN_START);
	// autoplay and label box
	GtkWidget *controlbar_label_box;
	controlbar_label_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
	gtk_box_prepend(GTK_BOX(controlbar_label_box), controlBar);
	gtk_box_append(GTK_BOX(controlbar_label_box), controlBarLabel);
	watchControlsSwitch = gtk_expression_watch(expressionStateSwitch, controlBar, notifyControlsSw,
																					controlBar, NULL);

	///////////////////////////////// extensions label
	GtkWidget *extLabel;
	extLabel = gtk_label_new("Add/Remove file extensions to be parsed");
	gtk_widget_set_halign(extLabel, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(extLabel, GTK_ALIGN_END);
	// entry buffer to hold file extensions from conf
	GtkEntryBuffer *extBuff;
	extBuff = gtk_entry_buffer_new(get_char_value_from_conf(pathToConfigDmelody, "formats"), -1);
	// entry to add or remove file extensions
	holdsExtensions = gtk_entry_new_with_buffer(extBuff);
	gtk_widget_set_size_request(holdsExtensions, 262, 0);
	// customizing the style, text size of the GtkEntry Widget
	PangoAttrList *attr_list = pango_attr_list_new();
	PangoAttribute *attr = pango_attr_size_new_absolute(15 * PANGO_SCALE);
	pango_attr_list_insert(attr_list, attr);
	PangoLayout *layout = gtk_widget_create_pango_layout(holdsExtensions, NULL);
	pango_layout_set_attributes(layout, attr_list);
	gtk_entry_set_attributes(GTK_ENTRY(holdsExtensions), attr_list);

	////////////////////////////// control bar
	gint width				= get_int_value_from_conf(pathToConfigDmelody, "controlWidth");
	gint height				= get_int_value_from_conf(pathToConfigDmelody, "controlHeight");
	gint posx				= get_int_value_from_conf(pathToConfigDmelody, "controlX");
	gint posy				= get_int_value_from_conf(pathToConfigDmelody, "controlY");

	// panel width
	GtkWidget *labelWidth;
	labelWidth = gtk_label_new("Control Bar Width");
	adjustmentSizeWidth = gtk_adjustment_new(width, 0, 700, 10, 1, 1);
	gtk_widget_set_halign(labelWidth, GTK_ALIGN_START);
	// entry width
	GtkWidget *entrySizeWidth;
	entrySizeWidth = gtk_spin_button_new(adjustmentSizeWidth, 1, 0);
	gtk_widget_set_halign(entrySizeWidth, GTK_ALIGN_START);
	// panel height
	GtkWidget *labelHeight;
	labelHeight = gtk_label_new("Control Bar Height");
	adjustmentSizeHeight = gtk_adjustment_new(height, 0, 700, 10, 1, 1);
	gtk_widget_set_halign(labelHeight, GTK_ALIGN_END);
	// entry
	GtkWidget *entrySizeHeight;
	entrySizeHeight = gtk_spin_button_new(adjustmentSizeHeight, 1, 0);
	gtk_widget_set_halign(entrySizeHeight, GTK_ALIGN_END);
	// width and height
	GtkWidget *boxWidthHeight;
	boxWidthHeight = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxWidthHeight), TRUE);
	gtk_box_prepend(GTK_BOX(boxWidthHeight), labelWidth);
	gtk_box_append(GTK_BOX(boxWidthHeight), entrySizeWidth);
	gtk_box_append(GTK_BOX(boxWidthHeight), labelHeight);
	gtk_box_append(GTK_BOX(boxWidthHeight), entrySizeHeight);

	// panel position x
	GtkWidget *labelX;
	labelX = gtk_label_new("Control Bar Pos X");
	adjustmentPosx = gtk_adjustment_new(posx, -4000, 4000, 10, 1, 1);
	gtk_widget_set_halign(labelX, GTK_ALIGN_START);
	// entry pos x
	GtkWidget *entryPosx;
	entryPosx = gtk_spin_button_new(adjustmentPosx, 1, 0);
	gtk_widget_set_halign(entryPosx, GTK_ALIGN_START);
	// panel position y
	GtkWidget *labelY;
	labelY = gtk_label_new("Control Bar Pos Y");
	adjustmentPosy = gtk_adjustment_new(posy, -4000, 4000, 10, 1, 1);
	gtk_widget_set_halign(labelY, GTK_ALIGN_END);
	// entry pos y
	GtkWidget *entryPosy;
	entryPosy = gtk_spin_button_new(adjustmentPosy, 1, 0);
	gtk_widget_set_halign(entryPosy, GTK_ALIGN_END);
	// position x and position y
	GtkWidget *boxPosition;
	boxPosition = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
	gtk_box_set_homogeneous(GTK_BOX(boxPosition), TRUE);
	gtk_box_prepend(GTK_BOX(boxPosition), labelX);
	gtk_box_append(GTK_BOX(boxPosition), entryPosx);
	gtk_box_append(GTK_BOX(boxPosition), labelY);
	gtk_box_append(GTK_BOX(boxPosition), entryPosy);

	// Close button
	GtkWidget *closeSettings;
	closeSettings = gtk_button_new_with_label("Close/Save");
	gtk_widget_set_halign(closeSettings, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(closeSettings, GTK_ALIGN_CENTER);
	g_signal_connect_swapped(closeSettings, "clicked", G_CALLBACK(save_close), windowSettings);

	// contains all widgets
	GtkWidget *allBox;
	allBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_set_homogeneous(GTK_BOX(allBox), TRUE);
	gtk_box_prepend(GTK_BOX(allBox), autoplay_label_box);
	gtk_box_append(GTK_BOX(allBox), minimiz_label_box);
	gtk_box_append(GTK_BOX(allBox), controlbar_label_box);
	gtk_box_append(GTK_BOX(allBox), extLabel);
	gtk_box_append(GTK_BOX(allBox), holdsExtensions);
	gtk_box_append(GTK_BOX(allBox), boxWidthHeight);
	gtk_box_append(GTK_BOX(allBox), boxPosition);
	gtk_box_append(GTK_BOX(allBox), closeSettings);
	gtk_widget_set_margin_start(allBox, 20);
	gtk_widget_set_margin_end(allBox, 20);
	gtk_widget_set_margin_top(allBox, 20);
	gtk_widget_set_margin_bottom(allBox, 20);

	g_object_unref(layout);
	pango_attr_list_unref(attr_list);

	gtk_window_set_child(GTK_WINDOW(windowSettings), allBox);
	gtk_window_present(GTK_WINDOW(windowSettings));
}
