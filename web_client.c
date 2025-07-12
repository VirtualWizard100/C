#include "chap05.h"

#define TIMEOUT 5.0

#define SHOW_HASH 0

void parse_url(char *url, char **hostname, char **port, char **path) {

	printf("URL: %s\n", url);

	char *p;

	p = strstr(url, "://");	//if :// exists in the url string, then return the beginning address of : in ://

	if (p) {
		printf("%s\n", p);	//previously explained comment shown here
	};

	char *protocol = 0;

	if (p) {		//If there was actually a :// in the url, then p won't be equal to 0, as explained above
		protocol = url;	//Now that we know that there is a :// in the url, then protocol will be equal to the beginning address of the whole url to see if http is before the :// as will be explained below
		printf("protocol before the NULL (:) : %s\n", protocol);
		*p = 0;		//set current char in p (:) to 0 to cut off the protocol string from the hostname string, so that way only the protocol name is stored in http
		p += 3;		//p moves 3 bytes foreward (past the ://) to get to the hostname
		printf("%s\n", p);	//print the hostname stored in p
		printf("protocol: %s\n", protocol);
	} else {
		p = url;	//if no :// was found in the url, then p will be equal to the whole url, because it would have to be just the hostname onward
		printf("%s\n", p);
	};

	if (protocol) {		//If the above if statement was true, then the beginning address of url should've been stored in protocol, making it not equal to 0
		if (strcmp(protocol, "http")) {		//If http isn't in the url stored in protocol
			fprintf(stderr, "Unknown protocol: %s, only http is supported\n", protocol);
			exit(1);
		};
	};

	*hostname = p;	//Now that p is for sure at the starting address of hostname, it will be stored in the variable hostname


	printf("hostname: %s\n", *hostname);

	while (*p && *p != ':' && *p != '/' && *p != '#') {	//keep p moving through the hostname until it reaches one of these characters, or the url ends, if the url doesn't end, then the next characters that's expected is "/" for the path, or ":" fro the port
		++p;
	};

	*port = "80";

	if (*p == ':') {
		*p++ = 0;	//null out the :, then move foreward 1 byte in the url
		*port = p;
	};

	printf("port before: %s\n", *port);

	while (*p && *p != '/' && *p != '#') {
		++p;
	};

	*path = p;

	if (*p == '/') {	 //Just to get the pesky "/" out of the path
		*path = p + 1;
	};

	*p++ = 0;		//null out the / to separate the port or hostname from the path

	while (*p && *p != '#') {	//keep moving through the url until you reach the end, or reach a '#'
		++p;
	};

	if (*p == '#') {
		*p = 0;		//null out the '#' to separate the path from the hash
	};

#if SHOW_HASH == 1
	p++;

	printf("Hash: %s\n", p);
#endif

	printf("hostname: %s\n", *hostname);
	printf("port: %s\n", *port);
	printf("path: %s\n", *path);
};

void send_request(SOCKET s, char *hostname, char *port, char *path) {	//This function is pretty self explanatory after the parse_url function, it also comes after the connect function, because we need to make the socket first

                char buffer[2048];

                sprintf(buffer, "GET /%s HTTP/1.1\r\n", path);
                sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
                sprintf(buffer + strlen(buffer), "Connection: close\r\n");
                sprintf(buffer + strlen(buffer), "User-Agent: honpwc web_get 1.0\r\n");
                sprintf(buffer + strlen(buffer), "\r\n");

                send(s, buffer, strlen(buffer), 0);

		printf("Sent Headers: \n%s", buffer);



};

SOCKET connect_to_host(char *hostname, char *port) {	//So we can connect to the client after recieving the hostname, and port back from the parse_url function
	struct addrinfo hints;
	struct addrinfo *peer_address;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname, port, &hints, &peer_address) < 0) {
		fprintf(stderr, "getaddrinfo() Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

	char address_buffer[100];
	char service_buffer[100];

	if (getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST) < 0) {
		fprintf(stderr, "getnameinfo() Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

	printf("Remote Peer Address: %s\nService: %s\n", address_buffer, service_buffer);

	SOCKET server;

	server = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);

	if (!ISVALIDSOCKET(server)) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

	if (connect(server, peer_address->ai_addr, peer_address->ai_addrlen) < 0) {
		fprintf(stderr, "connect() Failed: errno(%d)\n", GETERRNO());
		exit(1);
	};

	return server;
};

int main(int argc, char *argv[]) {

	if (argc < 2) {
		fprintf(stderr, "Usage: web_client url\n");
		return 1;
	};

	char *URL = argv[1];

	char *hostname, *port, *path;

	parse_url(URL, &hostname, &port, &path);

	SOCKET server = connect_to_host(hostname, port);

	//printf("%d\n", server);

	send_request(server, hostname, port, path);

	const clock_t start_time = clock();

#define RESPONSE_SIZE 8192

	char response[RESPONSE_SIZE+1];

	char *p = response, *q;		//Pointer to iterate through the response

	char *end = response + RESPONSE_SIZE;	//The end of the response

	char *body = 0;	//The beginning of the response body pointer

	enum {length, chunked, connection};	//length = 0, chunked = 1, connection = 2(The possible connection types in the Response Header)

	printf("%d, %d, %d\n", length, chunked, connection);

	int encoding = 0;

	int remaining = 0;

	while (1) {
		if ((clock() - start_time) / CLOCKS_PER_SEC > TIMEOUT) {
			fprintf(stderr, "timeout after %.2f seconds\n", TIMEOUT);
			return 1;
		};

		if (p == end) {
			fprintf(stderr, "out of buffer space\n");
			return 1;
		};



		fd_set reads;

		FD_ZERO(&reads);
		FD_SET(server, &reads);

		SOCKET max_socket;

		struct timeval timeout;

		timeout.tv_sec = 0;

		timeout.tv_usec = 2000000;

		if (select(server+1, &reads, 0, 0, &timeout) < 0) {
			fprintf(stderr, "select() Failed: errno(%d)\n", GETERRNO());
			return 1;
		};

		if (FD_ISSET(server, &reads)) {
			int bytes_recieved = recv(server, p, end-p, 0);	//The recieved information ends up in char pointer p to start moving through the response, the length of p is the address of the end minus the beginning address of p


			printf("Bytes Recieved: %d\n", bytes_recieved);

			if (bytes_recieved < 1) {
				if (encoding == connection && body) {	//If the body has already been assigned to the body of the data, and the current bytes_recieved is less than 1, and the encoding is equal to connection, instead of one of the two length encodings, then that means that the server is simply closing the connection, and we're just going to print whatever body we have recieved
					printf("%.*s\n", (int) (end-body), body);
				};
				printf("Connection Closed By Peer\n");
				break;
			};

			p += bytes_recieved;	//p is advanced to the end of the data to set the end byte to null terminator 0

			*p = 0;		//Nulling out the byte after the end of the response data to null terminate it

			if (!body && (body = strstr(response, "\r\n\r\n"))) { //First, it checks the initial value of body which is zero, so that condition is true, then it makes it's value the blank line's start address of the end of the response headers
				*body = 0;	//to cut off the \r\n\r\n from the rest of the previous response data
				body += 4;	//move p foreward 4 bytes to move past 0\n\r\n

				printf("Recieved Headers:\n%s\n", response);	//To print the newly cut off Response headers from the rest of the response data
			};


			q = strstr(response, "\nContent-Length: ");

			if (q) {	//If the previous assignment returned data. then we know that the response's encoding is equal to length, which means that all of the data from the http response is going to be in this current recv()
				encoding = length;
				printf("Encoding = Length\n");
				q = strchr(q, ' ');	//q will be equal to the space in Content-Length between the header name, and the value
				q += 1;			//To make q equal to the start address of the value in Content-Length
				remaining = strtol(q, 0, 10);	//convert the value in the address of q to decimal, and store it in remaining for the remaining Content-Length amount
			} else {		//If it didn't return data
				q = strstr(response, "\nTransfer-Encoding: chunked");	//Check to see if the encoding is equal to chunked
				if (q) {		//If the previous statement returned data
					encoding = chunked;	//encoding is equal to chunked
					remaining = 0;		//0 out the remaining amount
				} else {
					encoding = connection;	//If neither previous condition was true, then it must mean that the encoding is equal to connection
				};

				printf("\nRecieved Body: \n");
			};

			if (body) {
				if (encoding == length) {
					if  (p-body >= remaining) {	//If the end address of the bytes_recieved  - the beginning address of the body is greater than, or equal to the remaining amount, which will be the amount of bytes said in the Content-Length header
						printf("%p - %p >= %d\n", p, body, remaining);
						printf("%.*s", remaining, body);	//Print the remainin
						break;
					} else if (encoding == chunked) {
						do {
							if (remaining == 0) {
								if ((q = strstr(body, "\r\n"))) {
									remaining = strtol(body, 0, 16);
									if (!remaining) goto finish;
									body = q + 2;
								} else {
									break;
								};
							};

							if (remaining && p-body >= remaining) {
								printf("%.*s", remaining, body);
								body += remaining + 2;
								remaining = 0;
							};
						} while (!remaining);
					};
				};
			};	//if body


		};	//if FD_ISSET
	};	//while 1

finish:

	printf("\nClosing Socket..\n");
	CLOSESOCKET(server);

	return 0;

};

