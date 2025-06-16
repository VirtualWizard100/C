#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#define SOCKET int

int main(int argc, char *argv[]) {

	if (argc < 3) {
		fprintf(stderr, "Usage: SocketClient host_name port\n");
		return 1;
	};

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *host_address;

	if (getaddrinfo(argv[1], argv[2], &hints, &host_address)) {
		fprintf(stderr, "getaddrinfo() Failed: errno(%d)", errno);
		return 1;
	};

	printf("Host Address Is: ");

	char host_buffer[100];
	char service_buffer[100];

	if (getnameinfo(host_address->ai_addr, host_address->ai_addrlen, host_buffer, sizeof(host_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST)) {
		fprintf(stderr, "getnameinfo() Failed: errno(%d)\n", errno);
		return 1;
	};

	printf("%s %s\n", host_buffer, service_buffer);

	SOCKET peer_socket;

	peer_socket = socket(host_address->ai_family, host_address->ai_socktype, host_address->ai_protocol);	//using the host's family, socktype, and protocol to make sure that were always using whatever the host is using in our socket

	if (peer_socket < 0) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", errno);
		return 1;
	};

	if (connect(peer_socket, host_address->ai_addr, host_address->ai_addrlen)) { //Peer Socket connects to the server using it's IP Address and IP Address Length
		fprintf(stderr, "connect() Failed: errno(%d)\n", errno);
		return 1;
	};

	//peer socket is the accepted connection from the server

	freeaddrinfo(host_address);	//Now that Peer Socket is Connected to the Server that we want, we no longer need it's IP Address info, so we can free that struct

	printf("Connection Established\n");
	printf("To Send Data, Enter Text Followed By Enter\n");

	while (1) {

		fd_set reads; //A set of file descriptors that will be read from to send to the peer(server)
		FD_ZERO(&reads); //Zero out the memory space allocated for reads to begin putting fds in

		FD_SET(peer_socket, &reads); //Puts our connected socket into the reads fd_set
		FD_SET(0, &reads); //Adding stdin as a fd to fd_set reads to read stdin from terminal

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		if (select(peer_socket+1, &reads, 0, 0, &timeout) < 0) { //Blocks for stdin in terminal to be read, and sent to peer_socket (The Server), or for peer_socket (The Server) to send data to peer_socket (Us)
			fprintf(stderr, "select() Failed: errno(%d)\n", errno);
			return 1;
		};

		//Recieved data from peer_socket (The Server)

		if (FD_ISSET(peer_socket, &reads)) { //If the peer(server) returns something then select won't clear the fd socket_client from the fd_set reads, which means then you can read what the host sent
			char read[4096];
			int recieved = recv(peer_socket, read, 4096, 0);
			if (recieved < 1) {	//If the data recieved from the peer(server) is less than 1 byte in length, then that means that the peer(server) had closed connection
				printf("Socket Closed By Host\n");
				break;
			};

			printf("Recieved (%d) bytes: %.*s\n", (int) strlen(read), recieved, read);
		};

		//Sent data from stdin in terminal to peer_socket (The Server)

		if (FD_ISSET(0, &reads)) {
			char read[4096];
			if (!fgets(read, 4096, stdin)) { //If 0 is read from stdin (You just pressed Enter), then break from the if statement
				break;
			};

			printf("Sending: %s\n", read);
			int bytes_sent = send(peer_socket, read, (int) strlen(read), 0);

			if (bytes_sent < 0) {
				fprintf(stderr, "send() Failed: errno(%d)\n", errno);
				return 1;
			};

			printf("%d of %d bytes sent\n", bytes_sent, (int) strlen(read));

		};

	};

	close(peer_socket);
	printf("Connection Closed\n");
	return 0;

};
