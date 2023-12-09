#include <glib.h>
#include <gtk/gtk.h>

#ifndef EXTERNVARS_H_
#define EXTERNVARS_H_

#define APPNAME			popControlBar
#define VERSION			"DMelody 1.0"

	extern gboolean isPlaying;
	extern gboolean isWindowOpened;
	extern gboolean togglePlayPause;

	extern gint  appstate;
	extern gint  tabClicked;
	extern guint nrOfElements;
	extern gint  openDirState;
	extern gint	 addToPlaylistState;
	extern gint	 addToPlaylistState2;

	extern const gchar *HOME;
	extern const gchar *configDmelody;
	extern const gchar *configPlaylist1;
	extern const gchar *configPlaylist2;
	extern const gchar *configPlaylist3;
	extern const gchar *configDmelodyDir;

	extern gchar EXIFARTIST[256];
	extern gchar EXIFTITLE[256];
	extern gchar EXIFALBUM[256];
	extern gchar EXIFDURATION[256];
	extern gchar EXIFFILENAME[256];

	extern gchar **dirs;
	extern gchar pathToPlaylist1[];
	extern gchar pathToPlaylist2[];
	extern gchar pathToPlaylist3[];
	extern gchar pathToConfigDmelody[];
	extern gchar fullPathToSelDir[1024];
	extern gchar pathToConfigDmelodyDir[777];

	extern gchar defaultEmptyCover[777];
	extern gchar pathToPlaylist1Cover[777];
	extern gchar pathToPlaylist2Cover[777];
	extern gchar pathToPlaylist3Cover[777];
	extern gchar coverArtFullPath[2048];

	extern const gchar *nums;
	extern const gchar *arts;
	extern const gchar *ttls;
	extern const gchar *albms;
	extern const gchar *durs;

	extern char **strs_full_path_to_track;
	extern char **allNumbers;
	extern char **allArtists;
	extern char **allTitles;
	extern char **allAlbums;
	extern char **allDurations;

	extern const gchar *nums2;
	extern const gchar *arts2;
	extern const gchar *ttls2;
	extern const gchar *albms2;
	extern const gchar *durs2;

	extern const gchar *nums3;
	extern const gchar *arts3;
	extern const gchar *ttls3;
	extern const gchar *albms3;
	extern const gchar *durs3;

	extern gchar pathToConfigDmelodyDirNumbersFile[1024];
	extern gchar pathToConfigDmelodyDirArtistsFile[1024];
	extern gchar pathToConfigDmelodyDirTitlesFile[1024];
	extern gchar pathToConfigDmelodyDirAlbumsFile[1024];
	extern gchar pathToConfigDmelodyDirDurationsFIle[1024];

	extern gchar pathToConfigDmelodyDirNumbersFile2[1024];
	extern gchar pathToConfigDmelodyDirArtistsFile2[1024];
	extern gchar pathToConfigDmelodyDirTitlesFile2[1024];
	extern gchar pathToConfigDmelodyDirAlbumsFile2[1024];
	extern gchar pathToConfigDmelodyDirDurationsFIle2[1024];

	extern gchar pathToConfigDmelodyDirNumbersFile3[1024];
	extern gchar pathToConfigDmelodyDirArtistsFile3[1024];
	extern gchar pathToConfigDmelodyDirTitlesFile3[1024];
	extern gchar pathToConfigDmelodyDirAlbumsFile3[1024];
	extern gchar pathToConfigDmelodyDirDurationsFIle3[1024];

	extern GPid		  pid;
	extern GMenu	  *menu;
	extern GMenuModel *menuModel;
	extern GListModel *dirsListModel;
	extern GListModel *ListModelNumbers;

	extern GtkStringList *directories;
	extern GtkStringList *numbers;
	extern GtkStringList *artists;
	extern GtkStringList *titles;
	extern GtkStringList *albums;
	extern GtkStringList *durations;

	extern GtkColumnViewColumn *columnViewNums;
	extern GtkColumnViewColumn *columnViewArts;
	extern GtkColumnViewColumn *columnViewTitles;
	extern GtkColumnViewColumn *columnViewAlbums;
	extern GtkColumnViewColumn *columnViewDurations;

	extern GtkApplication	  *app;
	extern GtkAdjustment	  *adjScale;
	extern GtkSelectionModel  *selectionModel;
	extern GtkSingleSelection *singleSelectionDirs;
	extern GtkSingleSelection *singleSelectionNums;
	extern GtkExpression	  *expressionStateSwitch;

	extern GtkAdjustment	*adjustmentPosx;
	extern GtkAdjustment	*adjustmentPosy;
	extern GtkAdjustment	*adjustmentSizeWidth;
	extern GtkAdjustment	*adjustmentSizeHeight;

	extern GtkWidget *cover;
	extern GtkWidget *menuPop;
	extern GtkWidget *playlist1;
	extern GtkWidget *playlist2;
	extern GtkWidget *playlist3;
	extern GtkWidget *playPause;
	extern GtkWidget *next_track;
	extern GtkWidget *play_pause;
	extern GtkWidget *browserList;
	extern GtkWidget *timeElapsed;
	extern GtkWidget *repeatTrack;
	extern GtkWidget *waitingPopup;
	extern GtkWidget *showControls;
	extern GtkWidget *dmelodyWindow;
	extern GtkWidget *popControlBar;
	extern GtkWidget *previous_track;
	extern GtkWidget *browserScrolledWindow;
	extern GtkWidget *trackListScrolledWindow;

#endif
