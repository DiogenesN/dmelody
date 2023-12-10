/* looks for strings containing given substrings and write the found ones to a config file
 * arguments:
 * path to look strings in,
 * array of substrings that will filter the strings accordingly (e.g. a list of extensions),
 * path to conf file to write found results into
 * mode to open the conf file, e.g. "a" - append, "w" - to replace the current content
 * example:
 * const char *path = "/home/diogenes/Music/test";
 * const char *extensions[] = { ".mp3", ".flac", ".opus", NULL };
 * const char *pathToConf = "/home/diogenes/Music/test.conf";
 * const char *mode = "a";
 * get_strs_from_substrs_write_to_conf(path, extensions, pathToConf, mode);
 */
 
#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define PATH_MAX 4096

// Define a structure to hold file entries
typedef struct {
	char path[PATH_MAX];
} FileEntry;

// Comparison function for qsort
int compareEntries(const void *a, const void *b) {
	return strcmp(((const FileEntry *)a)->path, ((const FileEntry *)b)->path);
}

void get_strs_from_substrs_write_to_conf_recursive(const char *path, const char *substrings[],
																FileEntry *entries, int *count) {

	DIR *dir = opendir(path);
	if (!dir) {
		perror("Error opening directory");
		return;
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

		if (S_ISDIR(statbuf.st_mode)) {
			// Recursive call for directories
			get_strs_from_substrs_write_to_conf_recursive(fullpath, substrings, entries, count);
		}
		else if (S_ISREG(statbuf.st_mode)) {
			// Check if the substring is present in the filename
			for (int i = 0; substrings[i] != NULL; i++) {
				if (strstr(entry->d_name, substrings[i]) != NULL) {
					printf("Found: %s/%s\n", path, entry->d_name);
					snprintf(entries[*count].path, sizeof(entries[*count].path), "%s/%s", path, entry->d_name);
					(*count)++;
					break; // Break to the next entry once a match is found
				}
			}
		}
	}
	closedir(dir);
}

void get_strs_from_substrs_write_to_conf(const char *path, const char *substrings[],
											const char *pathToConf, const char *mode) {

	printf("Path: %s\n", path);

	// Allocate an array to store file entries
	FileEntry *entries = (FileEntry *)malloc(sizeof(FileEntry) * PATH_MAX);
	if (entries == NULL) {
		perror("Error allocating memory");
		return;
	}

	int count = 0;

	get_strs_from_substrs_write_to_conf_recursive(path, substrings, entries, &count);

	// Sort the entries alphabetically
	qsort(entries, count, sizeof(FileEntry), compareEntries);

	FILE *config = fopen(pathToConf, mode);
	if (config == NULL) {
		perror("Error opening conffile");
		free(entries); // Release allocated memory before returning
		return;
	}

	// Write sorted entries to the configuration file
	for (int i = 0; i < count; i++) {
		printf("Writing: %s\n", entries[i].path);
		fprintf(config, "%s\n", entries[i].path);
	}

	// Close the configuration file after writing
	fclose(config);

	// Free allocated memory
	free(entries);
}
