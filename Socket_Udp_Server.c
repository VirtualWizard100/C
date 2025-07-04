#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SOCKET int
#define GETERRNO() (errno)
#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)

int main(int argc, char *argv[]) {
	struct addrinfo hints;
	struct addrinfo *bind_address;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if(getaddrinfo(0, "8080", &hints, &bind_address) < 0) {
		fprintf(stderr, "getaddrinfo() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	char address_buffer[100];
	char service_buffer[100];

	getnameinfo(bind_address->ai_addr, bind_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);

	printf("Local Address: %s\nService: %s\n", address_buffer, service_buffer);

	SOCKET listening_socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

	if(!ISVALIDSOCKET(listening_socket)) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	if(bind(listening_socket, bind_address->ai_addr, bind_address->ai_addrlen) < 0) {
		fprintf(stderr, "bind() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	freeaddrinfo(bind_address);

	struct sockaddr_storage client_address;
	socklen_t client_address_length = sizeof(client_address);
	char read[1024];
	int bytes_recieved = recvfrom(listening_socket, read, 1024, 0, (struct sockaddr *) &client_address, &client_address_length);

	memset(&address_buffer, 0, sizeof(address_buffer));
	memset(&service_buffer, 0, sizeof(service_buffer));
	getnameinfo((struct sockaddr *) &client_address, client_address_length, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST | NI_NUMERICSERV);

	printf("Remote Address: %s\nService: %s\n", address_buffer, service_buffer);
	printf("Recieved: %s\n", read);

	CLOSESOCKET(listening_socket);

	return 0;
};
