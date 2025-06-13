#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>

#if !defined(IPV6_V6ONLY)
#define IPV6_V6ONLY 27
#endif

int main(int argc, char *argv[]) {

	struct addrinfo hints;
	struct addrinfo *address_to_bind;

	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(0, "8080", &hints, &address_to_bind) != 0) {
		printf("Failed to get Local Address Info: errno(%d)\n", errno);
		return 1;
	} else {
		printf("Successfully Got Local Address Info\n");
	};

	int listening_socket = socket(address_to_bind->ai_family, address_to_bind->ai_socktype, address_to_bind->ai_protocol);

	if (listening_socket < 0) {
		printf("Failed to create Listening Socket: errno(%d)\n", errno);
		return 1;
	} else {
		printf("Successfully Created Listening Socket\n");
	};

	int optval = 0;

	if (setsockopt(listening_socket, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &optval, sizeof(optval)) < 0) {
		printf("Failed to Set Socket Option: errno(%d)\n", errno);
		return 1;
	} else {
		printf("Successfully Set Socket Option\n");
	};

	if (bind(listening_socket, address_to_bind->ai_addr, address_to_bind->ai_addrlen) < 0) {
		printf("Failed to bind Local Address to Listening Socket: errno(%d)\n", errno);
		return 1;
	} else {
		printf("Successfully Binded Local Address To Listening Socket\n");
	};

	if (listen(listening_socket, 10) < 0) {
		printf("Failed to Listen with The Listening Socket: errno(%d)\n", errno);
		return 1;
	} else {
		printf("Listening Socket Currently Listening\n");
	};

	struct sockaddr_storage *Client_Address;

	socklen_t Client_Address_Length = sizeof(Client_Address);

	while (1) {

		int Accepted_Socket = accept(listening_socket, (struct sockaddr *) &Client_Address, &Client_Address_Length);

		int pid = fork(); //Creates a child process that brings the newly Accepted Socket Along with it to handle the new request

		if (pid == 0) {

			close(listening_socket);

			if (Accepted_Socket < 0) {
				printf("Failed to Accept Client Connection: errno(%d)\n", errno);
				return 1;
			} else {
				printf("Accepted Client Connection\n");
			}

			char Client_Request[1024];

			if (read(Accepted_Socket, &Client_Request, sizeof(Client_Request)) < 0) {
				printf("Failed to read Client Request: errno(%d)\n", errno);
				return 1;
			} else {
				printf("Client Request Accepted\n\n");
				printf("%s\n", Client_Request);
			};

			char Client_Numeric_Address[100];

			if (getnameinfo((struct sockaddr *) &Client_Address, Client_Address_Length, Client_Numeric_Address, sizeof(Client_Numeric_Address), 0, 0, NI_NUMERICHOST) < 0) {
				printf("Failed to retrieve Numeric Client Address: errno(%d)\n", errno);
				return 1;
			};

			printf("Client Address: %s\n", Client_Numeric_Address);

			char *message = "HTTP/1.1 200 OK\r\n" "Connection: close\r\n" "Content-Type: text/plain\r\n\r\n" "Local Time: ";

			int sent = send(Accepted_Socket, message, (int) strlen(message), 0);

			if (sent < 0) {
				printf("Failed to Send HTTP Response: errno(%d)\n", errno);
				return 1;
			} else {
				printf("Successfully sent %d of %d bytes\n", sent, strlen(message));
			};

			time_t _Time;
			time(&_Time);
			char *Local_Time = ctime(&_Time);

			memset(&sent, 0, sizeof(sent));

			sent = send(Accepted_Socket, Local_Time, (int) strlen(Local_Time), 0);

			if (sent < 0) {
				printf("Failed to send response: errno(%d)\n", errno);
				return 1;
			} else {
				printf("Successfully sent %d of %d bytes\n", sent, strlen(Local_Time));
			};

			close(Accepted_Socket);
			exit(0);
		};

		close(Accepted_Socket);

	};

	return 0;

};
