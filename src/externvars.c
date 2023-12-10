#include <glib.h>
#include <gtk/gtk.h>

gboolean isPlaying = FALSE;
gboolean isWindowOpened = TRUE;
gboolean togglePlayPause = TRUE;

gint  appstate = 0;
gint  tabClicked = 1;
gint  openDirState = 0;
guint nrOfElements = 0;
gint  addToPlaylistState = 0;
gint  addToPlaylistState2 = 0;

gchar **dirs;
gchar pathToPlaylist1[777];
gchar pathToPlaylist2[777];
gchar pathToPlaylist3[777];
gchar fullPathToSelDir[1024];
gchar pathToConfigDmelody[777];
gchar pathToConfigDmelodyDir[777];

gchar defaultEmptyCover[777];
gchar pathToPlaylist1Cover[777];
gchar pathToPlaylist2Cover[777];
gchar pathToPlaylist3Cover[777];
gchar coverArtFullPath[2048];

const gchar	*HOME;
const gchar	*configPlaylist1  = "/.config/dmelody/.playlist1";
const gchar	*configPlaylist2  = "/.config/dmelody/.playlist2";
const gchar	*configPlaylist3  = "/.config/dmelody/.playlist3";
const gchar	*configDmelody	  = "/.config/dmelody/dmelody.conf";
const gchar	*configDmelodyDir = "/.config/dmelody";

char **strs_full_path_to_track = NULL;
char **allNumbers = NULL;
char **allArtists = NULL;
char **allTitles = NULL;
char **allAlbums = NULL;
char **allDurations = NULL;

const gchar	*nums	= "/.numbers";
const gchar	*arts	= "/.artists";
const gchar	*ttls	= "/.titles";
const gchar	*albms	= "/.albums";
const gchar	*durs	= "/.durations";

const gchar	*nums2	= "/.numbers2";
const gchar	*arts2	= "/.artists2";
const gchar	*ttls2	= "/.titles2";
const gchar	*albms2	= "/.albums2";
const gchar	*durs2	= "/.durations2";

const gchar	*nums3	= "/.numbers3";
const gchar	*arts3	= "/.artists3";
const gchar	*ttls3	= "/.titles3";
const gchar	*albms3	= "/.albums3";
const gchar	*durs3	= "/.durations3";

gchar pathToConfigDmelodyDirNumbersFile[1024];
gchar pathToConfigDmelodyDirArtistsFile[1024];
gchar pathToConfigDmelodyDirTitlesFile[1024];
gchar pathToConfigDmelodyDirAlbumsFile[1024];
gchar pathToConfigDmelodyDirDurationsFIle[1024];

gchar pathToConfigDmelodyDirNumbersFile2[1024];
gchar pathToConfigDmelodyDirArtistsFile2[1024];
gchar pathToConfigDmelodyDirTitlesFile2[1024];
gchar pathToConfigDmelodyDirAlbumsFile2[1024];
gchar pathToConfigDmelodyDirDurationsFIle2[1024];

gchar pathToConfigDmelodyDirNumbersFile3[1024];
gchar pathToConfigDmelodyDirArtistsFile3[1024];
gchar pathToConfigDmelodyDirTitlesFile3[1024];
gchar pathToConfigDmelodyDirAlbumsFile3[1024];
gchar pathToConfigDmelodyDirDurationsFIle3[1024];

gchar EXIFARTIST[256];
gchar EXIFTITLE[256];
gchar EXIFALBUM[256];
gchar EXIFDURATION[256];
gchar EXIFFILENAME[256];

GPid	   pid;
GMenu	   *menu;
GMenuModel *menuModel;
GListModel *dirsListModel;
GListModel *ListModelNumbers;

GtkStringList *directories;
GtkStringList *numbers;
GtkStringList *artists;
GtkStringList *titles;
GtkStringList *albums;
GtkStringList *durations;

GtkColumnViewColumn *columnViewNums;
GtkColumnViewColumn	*columnViewArts;
GtkColumnViewColumn *columnViewTitles;
GtkColumnViewColumn *columnViewAlbums;
GtkColumnViewColumn *columnViewDurations;

GtkApplication		*app;
GtkAdjustment		*adjScale;
GtkSelectionModel	*selectionModel;
GtkSingleSelection	*singleSelectionDirs;
GtkSingleSelection	*singleSelectionNums;
GtkExpression		*expressionStateSwitch;

GtkAdjustment	*adjustmentPosx;
GtkAdjustment	*adjustmentPosy;
GtkAdjustment	*adjustmentSizeWidth;
GtkAdjustment	*adjustmentSizeHeight;

GtkWidget *cover;
GtkWidget *menuPop;
GtkWidget *popBox;
GtkWidget *playlist1;
GtkWidget *playlist2;
GtkWidget *playlist3;
GtkWidget *playPause;
GtkWidget *next_track;
GtkWidget *play_pause;
GtkWidget *browserList;
GtkWidget *timeElapsed;
GtkWidget *repeatTrack;
GtkWidget *showControls;
GtkWidget *waitingPopup;
GtkWidget *dmelodyWindow;
GtkWidget *popControlBar;
GtkWidget *previous_track;
GtkWidget *browserScrolledWindow;
GtkWidget *trackListScrolledWindow;

GtkWidget *label;
GtkWidget *showPlayer;
GtkWidget *hideControls;
GtkWidget *closeButton;
