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
	struct addrinfo *peer_address;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;

	if (argc < 3) {
		printf("Format: ./Socket_udp ip_address||dns port||service\n");
		return 1;
	};

	if(getaddrinfo(argv[1], argv[2], &hints, &peer_address) < 0) {
		fprintf(stderr, "getaddrinfo() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	char address_buffer[100];
	char service_buffer[100];

	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST | NI_NUMERICSERV);

	printf("Local Address: %s\nService: %s\n", address_buffer, service_buffer);

	SOCKET peer_socket = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);

	if(!ISVALIDSOCKET(peer_socket)) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	const char *message = "Oi lads";

	printf("Sending %s\n", message);

	int bytes_sent = sendto(peer_socket, message, strlen(message), 0, peer_address->ai_addr, peer_address->ai_addrlen);

	printf("%d bytes sent\n", bytes_sent);

	CLOSESOCKET(peer_socket);

	return 0;
};
