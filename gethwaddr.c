#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>
#include <string.h>


#define S int

char *get_errno_message() {
	char *errno_message = strerror(errno);

	static char buffer[100];

	sprintf(buffer, "errno(%d): %s", errno, errno_message);

	return buffer;

};

char *get_hwaddr(struct ifreq ifr) {

	static char buffer[ETH_ALEN+5];

	for (int i = 0; i < ETH_ALEN; i++) {
		sprintf(buffer + strlen(buffer), "%02x", (unsigned char) ifr.ifr_hwaddr.sa_data[i]);
		if (i < 5) {
			sprintf(buffer + strlen(buffer), ":");
		};
	};

	return buffer;

};

int main(int argc, char **argv) {

	S s = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);

	if (s < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 3), get_errno_message());
		return 1;
	};


	struct ifreq ifr;

	if (strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ) < 0) {
		fprintf(stderr, "Line %d: strncpy failed\n", (__LINE__ - 1));
		return 1;
	};

	if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	printf("Ethernet Address: %s\n", get_hwaddr(ifr));

	return 0;

};
