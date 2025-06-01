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
		printf("%p\n", address->ifa_next);
		printf("%s\n\n", address->ifa_name);
		unsigned short int family = address->ifa_addr->sa_family;
		printf("%s or: 0x%x\n---------------\n\n", family == AF_INET ? "IPv4" : "IPv6", family);
		address = address->ifa_next;

		continue;
	};
	printf("AF_INET value: %d\nAF_INET6 value: %d\n", AF_INET, AF_INET6);
	return 0;

};
