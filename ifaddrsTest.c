#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
	struct ifaddrs * addresses;
	getifaddrs(&addresses);
	struct ifaddrs * address = addresses;
	while (address->ifa_next != NULL) {
		printf("%s\n", address->ifa_name);
		address = address->ifa_next;
		continue;
	};

	printf("AF_INET value: %d\nAF_INET6 value: %d\n", AF_INET, AF_INET6);
	return 0;

};
