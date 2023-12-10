/* creates initial config directory and file */

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

void create_configs() {
	const char *HOME = getenv("HOME");

	if (HOME == NULL) {
		fprintf(stderr, "Unable to determine the user's home directory.\n");
		return;
	}

	char musicPath[777];
	char dirConfigBuff[777];
	char fileConfigBuff[777];
	char fileConfigBuff1[777];
	char fileConfigBuff2[777];
	char fileConfigBuff3[777];
	const char *dirConfig	= "/.config/dmelody";
	const char *fileConfig	= "/dmelody.conf";
	const char *fileConfig1	= "/.playlist1";
	const char *fileConfig2	= "/.playlist2";
	const char *fileConfig3	= "/.playlist3";

	snprintf(musicPath, sizeof(musicPath), "%s/Music", HOME);
	snprintf(dirConfigBuff, sizeof(dirConfigBuff), "%s%s", HOME, dirConfig);
	snprintf(fileConfigBuff, sizeof(fileConfigBuff), "%s%s%s", HOME, dirConfig, fileConfig);
	snprintf(fileConfigBuff1, sizeof(fileConfigBuff1), "%s%s%s", HOME, dirConfig, fileConfig1);
	snprintf(fileConfigBuff2, sizeof(fileConfigBuff2), "%s%s%s", HOME, dirConfig, fileConfig2);
	snprintf(fileConfigBuff3, sizeof(fileConfigBuff3), "%s%s%s", HOME, dirConfig, fileConfig3);

	DIR *confDir = opendir(dirConfigBuff);

	// cheks if the file already exists
	if (confDir) {
		// directory exists do nothing
		closedir(confDir);
		return;
	}
	else {
		// directory not found
		printf("created initial configs\n");
		mkdir(dirConfigBuff, 0755);
		closedir(confDir);
	}

	FILE *config = fopen(fileConfigBuff, "w+");
	FILE *config1 = fopen(fileConfigBuff1, "w+");
	FILE *config2 = fopen(fileConfigBuff2, "w+");
	FILE *config3 = fopen(fileConfigBuff3, "w+");

	fprintf(config, "%s\n", "width=700");
	fprintf(config, "%s\n", "height=700");
	fprintf(config, "%s\n", "autoplay=0");
	fprintf(config, "%s\n", "repeat=0");
	fprintf(config, "%s\n", "controlbar=0");
	fprintf(config, "%s\n", "formats=.mp3, .ogg, .oga, .m4a, .wav, .wma, .ape, .mogg, .opus, .flac, .alac, .aac, .ac3, .aiff, .ts, .dts");
	fprintf(config, "%s%s\n", "musicpath=", musicPath);
	fprintf(config, "%s\n", "playlist1name=Playlist 1");
	fprintf(config, "%s\n", "playlist2name=Playlist 2");
	fprintf(config, "%s\n", "playlist3name=Playlist 3");
	fprintf(config, "%s\n", "lastTabClicked=1");
	fprintf(config, "%s\n", "playlist1cover=/usr/local/share/icons/hicolor/32x32/apps/dmelody-image.png");
	fprintf(config, "%s\n", "playlist2cover=/usr/local/share/icons/hicolor/32x32/apps/dmelody-image.png");
	fprintf(config, "%s\n", "playlist3cover=/usr/local/share/icons/hicolor/32x32/apps/dmelody-image.png");
	fprintf(config, "%s\n", "startminimized=0");
	fprintf(config, "%s\n", "controlWidth=300");
	fprintf(config, "%s\n", "controlHeight=50");
	fprintf(config, "%s\n", "controlX=-50");
	fprintf(config, "%s\n", "controlY=-350");

	fprintf(config1, "%s", "");
	fprintf(config2, "%s", "");
	fprintf(config3, "%s", "");

	fclose(config1);
	fclose(config2);
	fclose(config3);
	fclose(config);
}
