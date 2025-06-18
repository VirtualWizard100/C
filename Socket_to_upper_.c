#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>

#define SOCKET int
#define MAX_QUEUE 10

int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *bind_address;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(0, "8080", &hints, &bind_address) < 0) {
		fprintf(stderr, "getaddrinfo() filaed: errno(%d)\n", errno);
		return 1;
	};

	char bind_address_buffer[100];
	char service_buffer[100];
	getnameinfo(bind_address->ai_addr, bind_address->ai_addrlen, bind_address_buffer, sizeof(bind_address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);

	printf("Local Address: %s\nService: %s\n", bind_address_buffer, service_buffer);

	SOCKET listening_socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

	if (listening_socket < 0) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", errno);
		return 1;
	};

	int option = 0;

	setsockopt(listening_socket, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &option, sizeof(option));

	if (option < 0) {
		fprintf(stderr, "setsockopt() Failed: errno(%d)\n", errno);
		return 1;
	};

	if (bind(listening_socket, bind_address->ai_addr, bind_address->ai_addrlen) < 0) {
		fprintf(stderr, "bind() Failed: errno(%d)\n", errno);
		return 1;
	};

	if (listen(listening_socket, MAX_QUEUE) < 0) {
		fprintf(stderr, "listen() Failed: errno(%d)\n", errno);
		return 1;
	};

	fd_set master;
	FD_ZERO(&master);
	FD_SET(listening_socket, &master);

	SOCKET max_socket = listening_socket;

	while (1) {
		fd_set reads;
		reads = master;

		if(select(max_socket+1, &reads, 0, 0, 0) < 0) {
			fprintf(stderr, "select() Failed: errno(%d)\n", errno);
			return 1;
		};

		SOCKET i;
		for (i = 0; i <= max_socket; ++i) {
			if (FD_ISSET(i, &reads)) {
				if (i == listening_socket) { //Each new connection is going to be on the listening_socket file descriptor
					struct sockaddr_storage client_address;
					socklen_t client_address_length = sizeof(client_address);
					SOCKET client_socket = accept(listening_socket, (struct sockaddr *) &client_address, &client_address_length); //Storing new Accepted connection file descriptor in master

					if (client_socket < 0) {
						fprintf(stderr, "accept() Failed: errno(%d)\n", errno);	
						return 1;
					};

					FD_SET(client_socket, &master);
					if (client_socket > max_socket) {
						max_socket = client_socket;
					};

					char address_buffer[100];

					getnameinfo((struct sockaddr *) &client_address, client_address_length, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);

					printf("New Connection From: %s\n", address_buffer);
				} else {	//An already Accepted Connection file descriptor has data
					char read[1024];
					int bytes_recieved = recv(i, read, sizeof(read), 0);

					if (bytes_recieved < 1) {
						FD_CLR(i, &master);
						close(i);
						printf("Connection Closed By Host\n");
						continue;
					};

					printf("Recieved %d of %d bytes\n", strlen(read), bytes_recieved);
					int j;
					for (j = 0; j < strlen(read); ++j) {
						read[j] = toupper(read[j]);
					};

					int bytes_sent = send(i, read, sizeof(read), 0);

					printf("Sent %d bytes\n", bytes_sent);
				};
			};
		};

	};


	printf("Closing Socket\n");
	close(listening_socket);
	return 0;
};
