#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "externvars.h"

void populate_allNumbers(char *pathToConfigDmelodyDirNumbersFile) {
	int lineCount = 0;
	char numbersBuff[100];
	allNumbers = (char **)malloc(sizeof(char *) * 3);

	FILE *numbersFile = fopen(pathToConfigDmelodyDirNumbersFile, "r");

	while (fgets(numbersBuff, sizeof(numbersBuff), numbersFile) != NULL) {
		// Remove the newline character if present
		size_t length = strlen(numbersBuff);
		if (length > 0 && numbersBuff[length - 1] == '\n') {
			numbersBuff[length - 1] = '\0';
		}

		// Check if the array needs to be resized
		if (lineCount > 0) {
			char **new_allNumbers = (char **)realloc(allNumbers,
									sizeof(char *) * (lineCount + 3));
			if (new_allNumbers == NULL) {
				// Handle realloc failure
				fprintf(stderr, "Memory reallocation failed\n");
				// Cleanup and exit
				fclose(numbersFile);
				for (int i = 0; i < lineCount; i++) {
					free(allNumbers[i]);
				}
				free(allNumbers);
				return;
			}
			allNumbers = new_allNumbers;
		 }

		// Allocate memory for the current line
		allNumbers[lineCount] = strdup(numbersBuff);

		if (allNumbers[lineCount] == NULL) {
			// Handle strdup failure
			fprintf(stderr, "Memory allocation failed\n");
			// Cleanup and exit
			fclose(numbersFile);
			for (int i = 0; i < lineCount; i++) {
				free(allNumbers[i]);
			}
			free(allNumbers);
			return;
		}
		// increasint line counter
		lineCount = lineCount + 1;
	}
	allNumbers[lineCount] = NULL;

	fclose(numbersFile);
}


void populate_allArtists(char *pathToConfigDmelodyDirArtistsFile) {
	int lineCount = 0;
	char artistsBuff[100];
	allArtists = (char **)malloc(sizeof(char *) * 3);

	FILE *artistsFile = fopen(pathToConfigDmelodyDirArtistsFile, "r");

	while (fgets(artistsBuff, sizeof(artistsBuff), artistsFile) != NULL) {
		// Remove the newline character if present
		size_t length = strlen(artistsBuff);
		if (length > 0 && artistsBuff[length - 1] == '\n') {
			artistsBuff[length - 1] = '\0';
		}

		// Check if the array needs to be resized
		if (lineCount > 0) {
			char **new_allArtists = (char **)realloc(allArtists,
									sizeof(char *) * (lineCount + 3));
			if (new_allArtists == NULL) {
				// Handle realloc failure
				fprintf(stderr, "Memory reallocation failed\n");
				// Cleanup and exit
				fclose(artistsFile);
				for (int i = 0; i < lineCount; i++) {
					free(allArtists[i]);
				}
				free(allArtists);
				return;
			}
			allArtists = new_allArtists;
		 }

		// Allocate memory for the current line
		allArtists[lineCount] = strdup(artistsBuff);

		if (allArtists[lineCount] == NULL) {
			// Handle strdup failure
			fprintf(stderr, "Memory allocation failed\n");
			// Cleanup and exit
			fclose(artistsFile);
			for (int i = 0; i < lineCount; i++) {
				free(allArtists[i]);
			}
			free(allArtists);
			return;
		}
		// increasint line counter
		lineCount = lineCount + 1;
	}
	allArtists[lineCount] = NULL;

	fclose(artistsFile);
}

void populate_allTitles(char *pathToConfigDmelodyDirTitlesFile) {
	int lineCount = 0;
	char titlesBuff[100];
	allTitles = (char **)malloc(sizeof(char *) * 3);

	FILE *titlesFile = fopen(pathToConfigDmelodyDirTitlesFile, "r");

	while (fgets(titlesBuff, sizeof(titlesBuff), titlesFile) != NULL) {
		// Remove the newline character if present
		size_t length = strlen(titlesBuff);
		if (length > 0 && titlesBuff[length - 1] == '\n') {
			titlesBuff[length - 1] = '\0';
		}

		// Check if the array needs to be resized
		if (lineCount > 0) {
			char **new_allTitles = (char **)realloc(allTitles,
									sizeof(char *) * (lineCount + 3));
			if (new_allTitles == NULL) {
				// Handle realloc failure
				fprintf(stderr, "Memory reallocation failed\n");
				// Cleanup and exit
				fclose(titlesFile);
				for (int i = 0; i < lineCount; i++) {
					free(allTitles[i]);
				}
				free(allTitles);
				return;
			}
			allTitles = new_allTitles;
		 }

		// Allocate memory for the current line
		allTitles[lineCount] = strdup(titlesBuff);

		if (allTitles[lineCount] == NULL) {
			// Handle strdup failure
			fprintf(stderr, "Memory allocation failed\n");
			// Cleanup and exit
			fclose(titlesFile);
			for (int i = 0; i < lineCount; i++) {
				free(allTitles[i]);
			}
			free(allTitles);
			return;
		}
		// increasint line counter
		lineCount = lineCount + 1;
	}
	allTitles[lineCount] = NULL;

	fclose(titlesFile);
}

void populate_allAlbums(char *pathToConfigDmelodyDirAlbumsFile) {
	int lineCount = 0;
	char albumsBuff[100];
	allAlbums = (char **)malloc(sizeof(char *) * 3);

	FILE *albumsFile = fopen(pathToConfigDmelodyDirAlbumsFile, "r");

	while (fgets(albumsBuff, sizeof(albumsBuff), albumsFile) != NULL) {
		// Remove the newline character if present
		size_t length = strlen(albumsBuff);
		if (length > 0 && albumsBuff[length - 1] == '\n') {
			albumsBuff[length - 1] = '\0';
		}

		// Check if the array needs to be resized
		if (lineCount > 0) {
			char **new_allAlbums = (char **)realloc(allAlbums,
									sizeof(char *) * (lineCount + 3));
			if (new_allAlbums == NULL) {
				// Handle realloc failure
				fprintf(stderr, "Memory reallocation failed\n");
				// Cleanup and exit
				fclose(albumsFile);
				for (int i = 0; i < lineCount; i++) {
					free(allAlbums[i]);
				}
				free(allAlbums);
				return;
			}
			allAlbums = new_allAlbums;
		 }

		// Allocate memory for the current line
		allAlbums[lineCount] = strdup(albumsBuff);

		if (allAlbums[lineCount] == NULL) {
			// Handle strdup failure
			fprintf(stderr, "Memory allocation failed\n");
			// Cleanup and exit
			fclose(albumsFile);
			for (int i = 0; i < lineCount; i++) {
				free(allAlbums[i]);
			}
			free(allAlbums);
			return;
		}
		// increasint line counter
		lineCount = lineCount + 1;
	}
	allAlbums[lineCount] = NULL;

	fclose(albumsFile);
}

void populate_allDurations(char *pathToConfigDmelodyDirDurationsFIle) {
	int lineCount = 0;
	char durationsBuff[100];
	allDurations = (char **)malloc(sizeof(char *) * 3);

	FILE *durationsFile = fopen(pathToConfigDmelodyDirDurationsFIle, "r");

	while (fgets(durationsBuff, sizeof(durationsBuff), durationsFile) != NULL) {
		// Remove the newline character if present
		size_t length = strlen(durationsBuff);
		if (length > 0 && durationsBuff[length - 1] == '\n') {
			durationsBuff[length - 1] = '\0';
		}

		// Check if the array needs to be resized
		if (lineCount > 0) {
			char **new_allDurations = (char **)realloc(allDurations,
									sizeof(char *) * (lineCount + 3));
			if (new_allDurations == NULL) {
				// Handle realloc failure
				fprintf(stderr, "Memory reallocation failed\n");
				// Cleanup and exit
				fclose(durationsFile);
				for (int i = 0; i < lineCount; i++) {
					free(allDurations[i]);
				}
				free(allDurations);
				return;
			}
			allDurations = new_allDurations;
		 }

		// Allocate memory for the current line
		allDurations[lineCount] = strdup(durationsBuff);

		if (allDurations[lineCount] == NULL) {
			// Handle strdup failure
			fprintf(stderr, "Memory allocation failed\n");
			// Cleanup and exit
			fclose(durationsFile);
			for (int i = 0; i < lineCount; i++) {
				free(allDurations[i]);
			}
			free(allDurations);
			return;
		}
		// increasint line counter
		lineCount = lineCount + 1;
	}
	allDurations[lineCount] = NULL;

	fclose(durationsFile);
}
