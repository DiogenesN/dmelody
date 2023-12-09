#include <stdio.h>

int get_number_of_lines(char *textfile) {
	FILE *file = fopen(textfile, "r");
	if (file == NULL) {
		perror("Error opening file");
		return 1;
	}

	int lineCount = 0;
	char buffer[1024]; // Adjust the buffer size as needed

	while (fgets(buffer, sizeof(buffer), file) != NULL) {
		lineCount++;
	}

	fclose(file);

	return lineCount;
}
