#include <stdio.h>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "address_struct_functions.h"

char *get_errno_message() {

	char *errno_message = strerror(errno);

	static char buffer[100];

	sprintf(buffer, "errno(%d): %s", errno, errno_message);

	return buffer;
};

int main(int argc, char **argv) {

	time_t start_time, current_time;

	time(&start_time);

	if (argc < 2) {
		fprintf(stderr, "Usage: main IPv6_Address\n");
		return 1;
	};

	int s = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);

	struct ifreq ifr;

	strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);

	if(ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	if(ioctl(s, SIOCGIFADDR, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	struct sockaddr_in addr;

	memcpy((void *) &(addr.sin_addr.s_addr), (void *) (ifr.ifr_addr.sa_data + 2), 4);

	if (memcmp((void *) &(addr.sin_addr.s_addr), (void *) (ifr.ifr_addr.sa_data + 2), 4)) {
		fprintf(stderr, "Line %d: addr.sin_addr.s_addr and ifr.ifr_addr.sa_data + 2 memory is not the same\n", (__LINE__ - 1));
		return 1;
	};

	printf("IPv4 Address: %s\n", ipv4_addrtoa(addr));

	if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	printf("Ethernet Address: %s\n", Ethernet_Address(ifr));

	struct sockaddr_in6 addr6;

	inet_pton(AF_INET6, argv[1], &(addr6.sin6_addr));

	printf("IPv6 Address: %s\n", ipv6_addrtoa(addr6));

	time(&current_time);

	double seconds = difftime(current_time, start_time);

	printf("Time elapsed: %2.2f\n", seconds);

	return 0;

};
