/* give it a string literal to be tokenized,
 * a token to split words in the string literal,
 * a charr array to be populated,
 * the size of the charr array,
 * e.g.:
 * const gchar *extStr = ".mp3, .flac, .opus";
 * const gchar *extArr[64];
 * chararr_from_strliteral_tok(extStr, ",", extArr, 64);
 * printf("extArr[0]=%s\n", extArr[0]);
 */

#include <stdio.h>
#include <string.h>

void chararr_from_strliteral_tok(const char *strlit, const char *delimiter,
								 const char *arr[], const int arrsize) {

	const char *token = strtok((char *)strlit, delimiter);
	int count = 0;

	while (token != NULL && count < arrsize) {
		arr[count++] = token;
		token = strtok(NULL, delimiter);
	}

	// null terminate the char array
	arr[count] = NULL;
}
