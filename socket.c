#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#define MAX_QUEUE 10

int main() {

	printf("\n");

	struct addrinfo hints;
	struct addrinfo *local_address_to_bind;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(0, "80", &hints, &local_address_to_bind) != 0) {
		printf("Failed to retrieve address info: errno(%d)\n", errno);
		return 1;
	} else {
		printf("Successfully Retrieved Local Address Info\n");
	};

	int listening_socket = socket(local_address_to_bind->ai_family, local_address_to_bind->ai_socktype, local_address_to_bind->ai_protocol);

	if (listening_socket < 0) {
		printf("Failed to create socket: errno(%d)\n", errno);
		return 1;
	} else {
		printf("Successfully Created Listening Socket Descriptor: (%d)\n", listening_socket);
	};

	if (bind(listening_socket, local_address_to_bind->ai_addr, local_address_to_bind->ai_addrlen) < 0) {
		printf("Failed to bind local address to listening socket: errno(%d)\n", errno);
		return 1;
	} else {
		printf("Successfully Binded Local Address To Listening Socket Descriptor\n");
	};

	listen(listening_socket, MAX_QUEUE);

	struct sockaddr_storage *Connected_Client_Address_Info;

	socklen_t Connected_Client_Address_Info_Length = sizeof(Connected_Client_Address_Info);

	int Accepted_Connection_Socket = accept(listening_socket, (struct sockaddr *) &Connected_Client_Address_Info, &Connected_Client_Address_Info_Length);

	if (Accepted_Connection_Socket < 0) {
		printf("Failed To Accept Client Connection: errno(%d)\n", errno);
		return 1;
	} else {
		printf("Successfully Accepted Client Connection\n");
	};

	char Client_Address[100];

	getnameinfo((struct sockaddr *) &Connected_Client_Address_Info, Connected_Client_Address_Info_Length, Client_Address, sizeof(Client_Address), 0, 0, NI_NUMERICHOST);

	printf("Client Address: %s\n", Client_Address);

	char Recieved_Request[1024];

	size_t Recieved_Request_Buffer_Length = sizeof(Recieved_Request);

	int Request_Length = read(Accepted_Connection_Socket, &Recieved_Request, Recieved_Request_Buffer_Length);

	printf("Successfully Recieved %d of %d bytes\n\n", Request_Length, (int) strlen(Recieved_Request));

	printf("%s\n", Recieved_Request);

	char *Response = "HTTP/1.1 200 OK\r\n" "Connection: close\r\n" "Content-Type: text/plain\r\n\r\n" "Local Time: ";

	int Bytes_Sent;

	Bytes_Sent = write(Accepted_Connection_Socket, Response, (int) strlen(Response));

	printf("Sent %d of %d bytes\n", Bytes_Sent, (int) strlen(Response));

	time_t _Time;

	time(&_Time);

	char *Local_Time_Buffer;

	Local_Time_Buffer = ctime(&_Time);

	memset(&Bytes_Sent, 0, sizeof(Bytes_Sent));

	Bytes_Sent = write(Accepted_Connection_Socket, Local_Time_Buffer, (int) strlen(Local_Time_Buffer));

	printf("Sent %d of %d bytes\n", Bytes_Sent, (int) strlen(Local_Time_Buffer));

	sync();

	close(Accepted_Connection_Socket);

	close(listening_socket);

	printf("\n");

	return 0;

};

