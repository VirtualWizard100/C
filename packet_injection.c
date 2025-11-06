#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/ether.h>
#include "Checksum.h"

#define SOCKET int
#define SOURCE_MAC_ADDR "00:bd:3e:41:b5:c4"
#define DEST_MAC_ADDR "aa:aa:aa:aa:aa:aa"

int main(int argc, char *argv[]) {

	SOCKET eth_s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	struct ifreq interface;

	strncpy(interface.ifr_name, "wlan0", IFNAMSIZ);

	ioctl(eth_s, SIOCGIFHWADDR, &interface);

	char buffer[65536];

	struct ethhdr *eth = (struct ethhdr *) buffer;

	memcpy(eth->h_source, (void *) ether_aton(SOURCE_MAC_ADDR), ETH_ALEN); //interface.ifr_hwaddr.sa_data, ETH_ALEN);

	memcpy(eth->h_dest, (void *) ether_aton(DEST_MAC_ADDR), ETH_ALEN);

	eth->h_proto = htons(0x0800);

	struct iphdr *ip = (struct iphdr *) (buffer + sizeof(struct ethhdr));

	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0b00000000;
	ip->tot_len = htons(sizeof(struct iphdr));
	ip->frag_off = 0;
	ip->ttl = 0x40;
	ip->protocol = 6;
	ip->saddr = inet_addr("192.168.0.46");
	ip->daddr = inet_addr("192.168.0.1");
	ip->check = Checksum((unsigned char *) ip, (ip->ihl * 4));

	SOCKET s = socket(AF_PACKET, SOCK_RAW, 0);

	struct sockaddr_ll sll;

	sll.sll_family = AF_PACKET;
	strncpy(sll.sll_addr, eth->h_source, ETH_ALEN);

	int sock_toggle = 1;

	ioctl(s, SIOCGIFINDEX, &interface);

	close(eth_s);

	struct sockaddr_ll sll_dest;

	strncpy(sll_dest.sll_addr, eth->h_dest, ETH_ALEN);
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = interface.ifr_ifindex;

	bind(s, (struct sockaddr *) &sll, sizeof(sll));

	while (1) {
		write(s, buffer, (sizeof(struct ethhdr) + sizeof(struct iphdr)));
//		sleep(1);
	};

	close(s);

};
