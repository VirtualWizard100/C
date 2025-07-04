#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>

#define SOCKET int
#define ISVALIDSOCKET(s) ((s) >= 0)
#define GETERRNO() (errno)
#define CLOSESOCKET(s) close(s)
#define MAX_QUEUE 10
#define TO_UPPER 1

int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *bind_address;
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(0, "8080", &hints, &bind_address) < 0) {
		fprintf(stderr, "getaddrinfo() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	char address_buffer[100];
	char service_buffer[100];

	if (getnameinfo(bind_address->ai_addr, bind_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST | NI_NUMERICSERV) < 0) {
		fprintf(stderr, "getnameinfo() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	printf("Local Address: %s\nService: %s\n\n", address_buffer, service_buffer);

	SOCKET listening_socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

	if (!ISVALIDSOCKET(listening_socket)) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	int optval = 0;

	if (setsockopt(listening_socket, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &optval, sizeof(optval)) < 0) {
		fprintf(stderr, "setsockopt() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	if (bind(listening_socket, bind_address->ai_addr, bind_address->ai_addrlen) < 0) {
		fprintf(stderr, "bind() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	if (listen(listening_socket, MAX_QUEUE) < 0) {
		fprintf(stderr, "listen() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	fd_set master;
	FD_ZERO(&master);
	FD_SET(listening_socket, &master);
	SOCKET max_socket = listening_socket;

	while (1) {
		fd_set reads;
		reads = master;

		if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
			fprintf(stderr, "select() Failed: errno(%d)\n", GETERRNO());
			return 1;
		};

		SOCKET i;

		for (i = 0; i <= max_socket; ++i) {
			if (FD_ISSET(i, &reads)) {
				if (i == listening_socket) {
					struct sockaddr_storage *client_address;
					socklen_t client_address_length = sizeof(client_address);

					SOCKET client_socket = accept(listening_socket, (struct sockaddr *) &client_address, &client_address_length);

					if (!ISVALIDSOCKET(client_socket)) {
						fprintf(stderr, "socket() in while (1) loop Failed: errno(%d)\n", GETERRNO());
						return 1;

					};

					FD_SET(client_socket, &master);

					if (client_socket > max_socket) {
						max_socket = client_socket;
					};

					char client_buffer[100];

					if (getnameinfo((struct sockaddr *) &client_address, client_address_length, client_buffer, sizeof(client_buffer), 0, 0, NI_NUMERICHOST) < 0) {
						fprintf(stderr, "getnameinfo() in while (1) loop Failed: errno(%d)\n", GETERRNO());
						return 1;
					};

					printf("New Connection From: %s\n", client_buffer);

				} else {

					char read[1024];

					int bytes_recieved = recv(i, read, 1024, 0);

					if (bytes_recieved < 1) {
						printf("Connection Closed By Client\n");
						FD_CLR(i, &master);
						close(i);
						continue;
					};

					printf("%d of %d bytes recieved\n", (int) strlen(read), bytes_recieved);

#if TO_UPPER == 1
					int c;

					for (c = 0; c < (int) strlen(read); c++) {
						read[c] = toupper(read[c]);
					};
#endif
					SOCKET j;

					for (j = 0; j <= max_socket; ++j) {
						if (FD_ISSET(j, &master)) {
							if (j == listening_socket || j == i) {
								continue;
							} else {

								int bytes_sent = send(j, read, (int) strlen(read), 0);

								printf("Sent %d of %d bytes\n", bytes_sent, (int) strlen(read));

							};
						};
					};

					memset(&read, 0, sizeof(read));

				};

			};

		};

	};


	printf("Finished\n");

	CLOSESOCKET(listening_socket);

	return 0;

};
