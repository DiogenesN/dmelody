/* control MPV playes using IPC server commands
 * commands MUST end wit newline character and the commands are:
 * playlist-prev
 * cycle pause
 * playlist-next
 * cycle-values loop-file "inf" "no" (toggle repeat)
 * quit
 * example:
	const char *socket_path = "/tmp/mpvsocket";
	const char *command = "quit\n";
	send_mpv_cmd(socket_path, command);
 */

#include <stdio.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void send_mpv_cmd(const char *socket_path, const char *command) {
	// Create a socket
	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sockfd == -1) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}

	// Set up the server address
	struct sockaddr_un server_addr;
	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

	// Connect to the server
	if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
		perror("Error connecting to socket");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	// Send the command
	ssize_t sent_bytes = send(sockfd, command, strlen(command), 0);
	if (sent_bytes == -1) {
		perror("Error sending command");
	}
    else {
		printf("Sent command: %s", command);
	}

	// Close the socket
	close(sockfd);
}
