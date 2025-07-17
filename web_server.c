                                                                                                
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define SOCKET int
#define ISVALIDSOCKET(s) ((s) >= 0)
#define GETERRNO() (errno)
#define CLOSESOCKET(s) close(s)

#define MAX_QUEUE 10
#define NAME_INFO 1

const char *get_content_type(const char *path) {

	const char *last_dot = strchr(path, '.');

	if (last_dot) {
		if (strcmp(last_dot, ".css") == 0) return "text/css";
		if (strcmp(last_dot, ".csv") == 0) return "text/csv";
		if (strcmp(last_dot, ".gif") == 0) return "image/gif";
		if (strcmp(last_dot, ".htm") == 0) return "text/html";
		if (strcmp(last_dot, ".html") == 0) return "text/html";
		if (strcmp(last_dot, ".ico") == 0) return "image/x-icon";
		if (strcmp(last_dot, ".jpeg") == 0) return "image/jpeg";
		if (strcmp(last_dot, ".jpg") == 0) return "image/jpeg";
		if (strcmp(last_dot, ".js") == 0) return "application/javascript";
		if (strcmp(last_dot, ".json") == 0) return "application/json";
		if (strcmp(last_dot, ".png") == 0) return "image/png";
		if (strcmp(last_dot, ".pdf") == 0) return "application/pdf";
		if (strcmp(last_dot, ".svg") == 0) return "image/svg+xml";
		if (strcmp(last_dot, ".txt") == 0) return "text/plain";
	};

	return "application/octet-stream";
};


SOCKET create_socket(char *hostname, char *port) {

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *bind_address;

	if (getaddrinfo(hostname, port, &hints, &bind_address) < 0) {
		fprintf(stderr, "getaddrinfo() Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

#if NAME_INFO == 1

	char address_buffer[100];
	char service_buffer[100];

	if (getnameinfo(bind_address->ai_addr, bind_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST | NI_NUMERICSERV) < 0) {
		fprintf(stderr, "getnameinfo() Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

	printf("Peer Address: %s\n", address_buffer);
	printf("Service: %s\n", service_buffer);

#endif

	SOCKET listening_socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

	if (!ISVALIDSOCKET(listening_socket)) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

	if (bind(listening_socket, bind_address->ai_addr, bind_address->ai_addrlen) < 0) {
		fprintf(stderr, "bind() Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

	freeaddrinfo(bind_address);

	if (listen(listening_socket, MAX_QUEUE) < 0) {
		fprintf(stderr, "listen Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

	return listening_socket;

};


#define MAX_REQUEST_SIZE 2047

struct client_info {
	socklen_t address_length;
	struct sockaddr_storage address;
	SOCKET socket;
	char request[MAX_REQUEST_SIZE+1];
	int recieved;
	struct client_info *next;
};

static struct client_info *clients = 0;		//static will keep the client_info datas across multiple function calls, a.k.a, it will never lose any connected clients data for as long as the program is running

struct client_info *get_client(SOCKET s) {
	struct client_info *ci = clients;	//make a copy of the clients info struct so we don't delete, or modify any of the original struct clients client data

	while (ci) {		//while there is struct data in ci
		if (ci->socket == s) {	//if the socket in the current struct is equal to s (the socket that we called with)
			break;
		};
		ci = ci->next;		//go to the next struct data
	};

	if (ci) {	//if ci has struct data in it
		return ci;		//if the socket has been found that's equal to the socket that we called with, return the struct data of that socket
	};

	struct client_info *n = (struct client_info *) calloc(1, sizeof(struct client_info));	//allocate 1 set of memory with size equal to struct client_info, and return the address of the newly allocated memory, and cast a struct client_info pointer to it

	if (!n) {		//if there wasn't enough memory to allocate memory for the new data structure, then n was never initialized
		fprintf(stderr, "Out of memory\n");
		exit(1);
	};

	n->address_length = sizeof(n->address);		//setting the address_length to the size of struct sockaddr_storage address

	n->next = clients;				//setting the current data structure *next pointer to the current beginning data structure address of root clients linked list

	clients = n;					//setting root clients to n, this accomplishes the task of adding the newly created data structure to the beginning of the linked list, since the root clients is static, none of the previously allocated data structures get deleted, this one just gets added to the beginning of the linked list

	return n;					//return the newly created data structure
};


void drop_client(struct client_info *client) {
	CLOSESOCKET(client->socket);

	struct client_info **p = &clients;	//p is a pointer to pointer variable that points to the linked list clients, p can be used to modify the clients linked list directly

	while (*p) {
		if (*p == client) {		//if the current data structure stored in p is equal to the client data structure that we are working on
			*p = client->next;	//That data structure is going to be equal to the next data structure, essentially overwriting the data structure with the next one, and removing it from the linked list (we overwrite the beginning address of the current p data structure with the beginning address of the next data stuctyre, essentially overwriting the data structure with the next one)
			free(client);		//Then we can free the allocated memory for the client data structure
			return;
		};
		p = &(*p)->next;		//else, p will be equal to the address of dereferenced p->next data structure to move on to the next data structure, dereferencing it when recieving the returned memory address
	};

	fprintf(stderr, "client not found\n");	//if p iterated through all of the data structures in the linked list, and it's address wasn't equal to the client that we are working on, then the client isn't in the linked list
	exit(1);
};

const char *get_client_address(struct client_info *ci) {
	static char address_buffer[100];

	getnameinfo((struct sockaddr *)&ci->address, ci->address_length, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);	//the first arguement is the address of the dereferenced ci->address struct cast as a dereferenced struct sockaddr pointer

	return address_buffer;
};

fd_set wait_on_clients(SOCKET server) {
	fd_set reads;
	FD_ZERO(&reads);
	FD_SET(server, &reads);

	SOCKET max_socket = server;

	struct client_info *ci = clients;

	while (ci) {	//while there's a data structure stored in ci
		FD_SET(ci->socket, &reads);	//add the socket in the current data structure of the ci linked list into fd_set reads
		if (ci->socket > max_socket) {	//if socket in the current data structure is greater than max_socket
			max_socket = ci->socket;//max_socket is equal to the socket
		};
		ci = ci->next;	//dereferenced data structure pointer ci is equal to the next data structure in the ci linked list
	};

	if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
		fprintf(stderr, "select() Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

	return reads;	//return the socket in reads that had an event happen on it

};

void send_400(struct client_info *client) {
		char *c400 = "HTTP/1.1 400 Bad Request\r\n" "Connection: close\r\n" "Content-Length: 11\r\n\r\nBad Request";

		send(client->socket, c400, strlen(c400), 0);

		drop_client(client);

};

void send_404(struct client_info *client) {
	const char *c404 = "HTTP/1.1 404 Not Found\r\n" "Connection: close\r\n" "Content-Length: 9\r\n\r\nNot Found";

	send(client->socket, c404, strlen(c404), 0);

	drop_client(client);
};

void serve_resource(struct client_info *client, const char *path) {

	printf("Serve resource %s %s\n", get_client_address(client), path);

	if (strcmp(path, "/") == 0) {
		path = "/index.html";
	};

	if (strlen(path) > 100) {
		send_400(client);
		return;
	};

	if (strstr(path, "..")) {
		send_404(client);
		return;
	};

	char full_path[128];

	sprintf(full_path, "public%s", path);

	FILE *fp = fopen(full_path, "rb");

	if (!fp) {		//if the path was not found locally
		send_404(client);
		return;
	};

	fseek(fp, 0L, SEEK_END);		//sets the current position of dereferenced file pointer fp to the end of the file

	size_t cl = ftell(fp);			//returns the current file position of fp, which is the amount of bytes that it is in the file, effectively giving us the file size

	printf("Current file position of fp in bytes: %d\n", cl);	//for debugging purposes

	rewind(fp);				//sets fp back to the beginning position of the file

	const char *ct = get_content_type(full_path);	//will return the Content-Type of the full_path of the file

#define BSIZE 1024

	char buffer[BSIZE];

	sprintf(buffer, "HTTP/1.1 200 OK\r\n");

	send(client->socket, buffer, strlen(buffer), 0);

	sprintf(buffer, "Connection: close\r\n");

	send(client->socket, buffer, strlen(buffer), 0);

	sprintf(buffer, "Content-Length: %u\r\n", cl);

	send(client->socket, buffer, strlen(buffer), 0);

	sprintf(buffer, "Content-Type: %s\r\n", ct);

	send(client->socket, buffer, strlen(buffer), 0);

	sprintf(buffer, "\r\n");

	send(client->socket, buffer, strlen(buffer), 0);

	int r = fread(buffer, 1, BSIZE, fp);		//fread returns the int byte length read to r from fp, and stores those bytes in buffer

	while (r) {					//while there is an int byte length in r
		send(client->socket, buffer, r, 0);	//will keep sending the client the data from the local path in 1024 byte increments until r = 0
		r = fread(buffer, 1, BSIZE, fp);	//read the next 1024 byte increment of the local path
	};

	fclose(fp);

	drop_client(client);

};

int main() {

	SOCKET server = create_socket(0, "8080");

	while (1) {
		fd_set reads;

		reads = wait_on_clients(server);				//once a client has connected, the socket fd with an event on it will return in the fd_set reads in the function, then we will make this fd_set reads equal to that one

		if (FD_ISSET(server, &reads)) {				//if an event happened on socket reads, then it's a new connection
			struct client_info *client = get_client(-1);	//-1 isn't a valid format specifier, so ci in get_client() is going to find -1 in the linked list clients, so a new client_info struct n is going to be created, and returned

			client->socket = accept(server, (struct sockaddr *) &(client->address), &(client->address_length));	//once it returns the newly initialized struct client_info into client, we can now store the newly accepted connection into struct sockaddr_storage *client->address cast as struct sockaddr, then put the struct sockaddr_storage length as the third arguement to store the new client information into the client_info->sockaddr_storage struct, and store the newly accepted connection socket in the client_info->socket type

			if (!ISVALIDSOCKET(client->socket)) {
				fprintf(stderr, "accept() Failed: errno(%d)\n", GETERRNO());
				return 1;
			};

			printf("New connection from %s\n", get_client_address(client));

		};

		struct client_info *client = clients;

		while (client) {

			struct client_info *next = client->next;

			if (FD_ISSET(client->socket, &reads)) {
				if (MAX_REQUEST_SIZE == client->recieved) {	//If the client has already sent 2047 bytes of data, the amount of bytes recieved from the client so far will be stored in int recieved
					send_400(client);
					client = next;
					continue;
				};

				int r = recv(client->socket, client->request + client->recieved, MAX_REQUEST_SIZE - client->recieved, 0);	//When we recieve the data, we store it at the offset in request equal to the amount of bytes recieved from the client before (client->request + client->recieved), and then give the amount of space equal to the MAX_REQUEST_SIZE - the amount of bytes already recieved from the client client->recieved

				printf("%.*s\n", r, client->request);

				if (r < 1) {
					printf("Unexpected disconnect from %s\n", get_client_address(client));
					drop_client(client);
				} else {
					client->recieved += r;
					client->request[client->recieved] = 0;	//go to the offset in client->request equal to the amount of bytes already recieved from the client and add a null terminator

					char *q = strstr(client->request, "\r\n\r\n");	//go to the \r\n\r\n dividing the request headers from the body

					if (q) {					//if q was able to find the \r\n\r\n
						*q = 0;

						if (strncmp("GET /", client->request, 5)) {	//if GET / is not in client->request
							send_400(client);
						} else {
							char *path = client->request + 4;	//after "GET "
							printf("Path: %s\n", path);
							char *end_path = strstr(path, " ");
							if (!end_path) {	//if there's no end to the path
								send_400(client);
							} else {
								*end_path = 0;		//the " " byte stored at the address of path will be equal to 0
								serve_resource(client, path);
							};
						};
					};
				};
			};

			client = next;

		};	//while (client)

	};	//while (1)

	printf("Closing socket\n");
	CLOSESOCKET(server);

	return 0;

};
