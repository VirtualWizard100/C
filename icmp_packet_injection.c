#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/icmp.h>
#include <errno.h>
#include <string.h>
#include "../C-Programming/C-Networking/C/Checksum.h"

#define SOCKET int
#define DEST_ADDR "aa:aa:aa:aa:aa:aa"
#define PROTO 1

int main(int argc, char **argv) {

	if (argc < 2) {
		fprintf(stderr, "Usage: icmp_packet_injection Destination_IP_Address\n");
		return 1;
	};

	char *errno_message;

	SOCKET s = socket(AF_PACKET, SOCK_RAW, IPPROTO_ICMP);

	if (s < 0) {
		errno_message = strerror(errno);
		fprintf(stderr, "socket() Failed: errno(%d) %s\n", errno, errno_message);
		return 1;
	};

	struct sockaddr_ll addr;
	memset(&addr, 0, sizeof(addr));

	addr.sll_family = AF_PACKET;
	addr.sll_protocol = htons(1);

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));

	strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);

	ioctl(s, SIOCGIFADDR, &ifr);

	memcpy((void *) (addr.sll_addr), (void *) (ifr.ifr_addr.sa_data + 2), 4);

	if(memcmp((void *) (addr.sll_addr), (void *) (ifr.ifr_addr.sa_data + 2), 4)) {
		fprintf(stderr, "addr.sll_addr, and ifr.ifr_addr.sa_data's memory is not the same\n");
		return 1;
	};

	printf("%u.%u.%u.%u\n", addr.sll_addr[0], addr.sll_addr[1], addr.sll_addr[2], addr.sll_addr[3]);

	ioctl(s, SIOCGIFINDEX, &ifr);

	addr.sll_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		errno_message = strerror(errno);
		fprintf(stderr, "bind() Failed: errno(%d) %s\n", errno, errno_message);
		return 1;
	};

	char *data = "Test data";

	char buffer[65536];

	//Ethernet Header

	struct ethhdr *eth = (struct ethhdr *) buffer;

	ioctl(s, SIOCGIFHWADDR, &ifr);

	memcpy((void *) (eth->h_source), (void *) (ifr.ifr_hwaddr.sa_data), ETH_ALEN);

	if (memcmp((void *) (eth->h_source), (void *) (ifr.ifr_hwaddr.sa_data), ETH_ALEN)) {
		fprintf(stderr, "eth->h_source, and ifr.ifr_hwaddr.sa_data's memory is not the same\n");
		return 1;
	};

	memcpy((void *) (eth->h_dest), (void *) ether_aton(DEST_ADDR), ETH_ALEN);
	eth->h_proto = htons(0x0800);

	//IP Header

	struct iphdr *ip = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0;
	ip->frag_off = 0x40;
	ip->saddr = (uint32_t) (192 | (168 << 8) | (0 << 16) | (6 << 24));
	ip->daddr = (uint32_t) (192 | (168 << 8) | (0 << 16) | (1 << 24));
	ip->protocol = 1;
	ip->id = htons(111);
	ip->ttl = 0x40;

	//ICMP Header

	struct icmphdr *icmp = (struct icmphdr *) (buffer + sizeof(struct ethhdr) + (ip->ihl * 4));

	icmp->type = 8;	//Echo
	icmp->code = 8;	//Request
	icmp->un.echo.id = getpid();

	sprintf((buffer + sizeof(struct ethhdr) + (ip->ihl * 4) + sizeof(struct icmphdr)), "%s", data);

	icmp->checksum = Checksum((unsigned char *) (buffer + sizeof(struct ethhdr) + (ip->ihl * 4)), sizeof(struct icmphdr) + strlen(data));
	ip->tot_len = htons(((ip->ihl * 4) + sizeof(struct icmphdr) + strlen(data)));
	ip->check = Checksum((unsigned char *) ip, (ip->ihl * 4));
/*
	char *quench_buffer = (buffer + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct icmphdr));

	strncpy(quench_buffer, (unsigned char *) ip, sizeof(ip));

	sprintf((buffer + sizeof(ip)), "%s", data);
*/
#if PROTO == 1

	socklen_t addrlen = sizeof(addr);
#endif

	while (1) {

#if PROTO == 1
		if (sendto(s, buffer, (sizeof(struct ethhdr) + (ip->ihl * 4) + sizeof(struct icmphdr) + strlen(data)), 0, (struct sockaddr *) &addr, addrlen) < 0) {
			errno_message = strerror(errno);
			fprintf(stderr, "Line %d: sendto() Failed: errno(%d) %s\n", __LINE__, errno, errno_message);
			return 1;
		};
#else
		write(s, buffer, (sizeof(struct ethhdr) + (ip->ihl *4) + sizeof(struct icmphdr) + strlen(data)));
#endif
		struct sockaddr_storage *address;

		socklen_t addrlen = sizeof(address);

		char recvfrom_buffer[65536];

		recvfrom(s, buffer, sizeof(buffer), 0, (struct sockaddr *) &address, &addrlen);
	};

	return 0;

};

