#include "chap05.h"

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

	while (*p && *p != ':' && *p != '/' && *p != '#') {	//keep p moving through the hostname until it reaches one of these characters, or the url ends, if the url doesn't end, then the next character that's expected is "/" for the path
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

	printf("hostname: %s\n", *hostname);
	printf("port: %s\n", *port);
	printf("path: %s\n", *path);
};

int main(int argc, char *argv[]) {
	char *URL = argv[1];

	char *hostname, *port, *path;
	parse_url(URL, &hostname, &port, &path);

	return 0;
};
