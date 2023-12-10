/* give a path and a key word and it returns the first occurrence 
 * of the filename (not directory) that constains the substring key word
 * e.g.:
 * const char *path = "/usr/share/icons";
 * const char *substring = "mousepad";
 * const char *result = get_filename_containing_substring(directoryPath, substring); 
 * printf("result: %s\n", result);
 * returns: /usr/share/icons/Lyra-blue-dark/apps@2x/scalable/mousepad.svg
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define PATH_MAX 4096

const char *get_filename_containing_substring(const char *path, const char *substring) {
	DIR *dir = opendir(path);
	if (!dir) {
		perror("Error opening directory");
		return NULL;
	}

	struct dirent *entry;
	struct stat statbuf;

	while ((entry = readdir(dir)) != NULL) {
		// Ignore current and parent directories
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		char fullpath[PATH_MAX];
		snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

		if (stat(fullpath, &statbuf) == -1) {
			// Ignore "No such file or directory" errors
			if (errno == ENOENT) {
				continue;
			}
			perror("Error getting file status");
			continue;
		}

		// if it's a directory, search inside of it
		if (S_ISDIR(statbuf.st_mode)) {
			const char *result = get_filename_containing_substring(fullpath, substring);
			if (result != NULL) {
				closedir(dir);
				return result;
			}
		}
		// if it's a regular file and contains the substring, return its full path
		else if (S_ISREG(statbuf.st_mode) && strcasestr(entry->d_name, substring) != NULL) {
			char *result = strdup(fullpath);
			closedir(dir);
			return result;
		}
	}
	closedir(dir);
	return NULL;
}
