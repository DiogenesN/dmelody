/* this function returns a value from JSON and used for MPV IPC
 * example:
 * (in BASH the command is:
 	echo '{ "command": ["get_property", "playlist-pos"] }' | socat - /tmp/mpvsocket

 * using this function:
 	const char *property_tag = "get_property";
 	const char *property_name = "playlist-pos";
    const char *key = "data"; // this will extract exact data
	int value = get_int_value_from_json(property_tag, property_name, key);
	(this returns the position of the currently playing track in the playlist
 */

#include <ctype.h>
#include <stdio.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAX_RESPONSE_SIZE 1024
#define MPV_SOCKET_PATH "/tmp/mpvsocket"

// Function to find the value of a key in a JSON-like string
int get_int_value_from_json(const char *property_tag, const char *property_name, const char *key) {
	int sockfd;
	int returned_value;
	struct sockaddr_un server_addr;
	char json_str[MAX_RESPONSE_SIZE];

	// Create a socket
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("Error creating socket");
		return EXIT_FAILURE;
	}

	// Set up the server address
	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, MPV_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

	// Connect to the server
	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("Error connecting to socket");
		close(sockfd);
		return EXIT_FAILURE;
	}

	// Send the command
	char mpv_full_command[512];
	const char *mpv_build_command[] = {"{ \"command\": [ ", property_tag, property_name, " ] }"};

	snprintf(mpv_full_command, sizeof(mpv_full_command), "%s\"%s\", \"%s\"%s\n",
															  mpv_build_command[0],
															  mpv_build_command[1],
															  mpv_build_command[2],
															  mpv_build_command[3]);
	//printf("command: %s", mpv_full_command);

	if (send(sockfd, mpv_full_command, strlen(mpv_full_command), 0) == -1) {
		perror("Error sending command");
		close(sockfd);
		return EXIT_FAILURE;
	}

	// Receive the JSON response
	ssize_t received_bytes;
	if ((received_bytes = recv(sockfd, json_str, sizeof(json_str) - 1, 0)) == -1) {
		perror("Error receiving response");
		close(sockfd);
		return EXIT_FAILURE;
	}

	// Null-terminate the received data
	json_str[received_bytes] = '\0';

	// Close the socket
	close(sockfd);

	const char *ptr = strstr(json_str, key);
	if (ptr == NULL) {
		fprintf(stderr, "Key '%s' not found in JSON response.\n", key);
		return -1; // Key not found
	}

	// Move to the end of the key
	ptr += strlen(key);

	// Skip whitespace
	while (*ptr != '\0' && !isdigit(*ptr)) {
		ptr++;
	}

	// Extract the integer value
	returned_value = atoi(ptr);

	return returned_value;
}
