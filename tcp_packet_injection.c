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

#define LOCAL_MAC_ADDR 1
#define LOCAL_IP_ADDR 1

#define SOCKET int
#define SRC_MAC_ADDR "aa:aa:aa:aa:aa:aa"
#define DEST_MAC_ADDR "bb:bb:bb:bb:bb:bb"

int main(int argc, char *argv[]) {

	SOCKET eth_s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	struct ifreq interface;

	strncpy(interface.ifr_name, "wlan0", IFNAMSIZ);

	ioctl(eth_s, SIOCGIFHWADDR, &interface);

	char buffer[65536];

	struct ethhdr *eth = (struct ethhdr *) buffer;

#if LOCAL_MAC_ADDR == 1 && LOCAL_IP_ADDR == 1

	memcpy(eth->h_source, (void *) interface.ifr_hwaddr.sa_data, ETH_ALEN);

#else
	memcpy(eth->h_source, (void *) ether_aton(SRC_MAC_ADDR), ETH_ALEN); //interface.ifr_hwaddr.sa_data, ETH_ALEN);
#endif
	memcpy(eth->h_dest, (void *) ether_aton(DEST_MAC_ADDR), ETH_ALEN);

	eth->h_proto = htons(0x0800);

	struct iphdr *ip = (struct iphdr *) (buffer + sizeof(struct ethhdr));

	ioctl(eth_s, SIOCGIFADDR, &interface);

	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0b00000000;

	char *data = 0;

	if (argc > 1) {
		data = argv[1];
	};

	ip->tot_len = htons((sizeof(struct iphdr) + strlen(data)));
	ip->frag_off = 0;
	ip->ttl = 0x40;
	ip->protocol = 6;
#if LOCAL_MAC_ADDR == 1 && LOCAL_IP_ADDR == 1
	unsigned char src_addr[16];
	for (int i = 0; i < sizeof(interface.ifr_addr.sa_data); i++) {
		if (i > 1) {
			src_addr[i-2] = interface.ifr_addr.sa_data[i];
		};
	};
	printf("%d.%d.%d.%d\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3]);
	ip->saddr = ((uint32_t) src_addr[3] << 24 | (uint32_t) src_addr[2] << 16 | (uint32_t) src_addr[1] << 8 | (uint32_t) src_addr[0]);
#else

	ip->saddr = inet_addr("192.168.0.46");
#endif
	ip->daddr = inet_addr("192.168.0.1");
	ip->check = Checksum((unsigned char *) ip, (ip->ihl * 4));

	struct tcphdr *tcp = (struct tcphdr *) (buffer + sizeof(struct ethhdr) + (ip->ihl * 4));

	tcp->source = htons(8080);
	tcp->dest = htons(54680);
	tcp->seq = htonl(111);
	tcp->ack_seq = htonl(111);
	tcp->res1 = 0;
	tcp->doff = (sizeof(struct tcphdr) / 4);
	tcp->syn = 1;
	tcp->window = htons(65535);

	struct Pseudoheader {
		struct in_addr src_addr;
		struct in_addr dest_addr;
		uint8_t reserved;
		uint8_t protocol;
		uint16_t segment_length;
	};

	struct Pseudoheader psdohdr;

	psdohdr.src_addr.s_addr = ip->saddr;
	psdohdr.dest_addr.s_addr = ip->daddr;
	psdohdr.reserved = 0;
	psdohdr.protocol = ip->protocol;

	unsigned int len = ntohs(ip->tot_len) - (ip->ihl * 4);

	unsigned int segment_len = len + sizeof(psdohdr);

	psdohdr.segment_length = htons(segment_len);

	printf("%d\n", len);
	tcp->check = Checksum((unsigned char *) &psdohdr, sizeof(psdohdr));

	SOCKET s = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);

//	int ip_toggle = 1;

//	setsockopt(s, IPPROTO_RAW, IP_HDRINCL, &ip_toggle, sizeof(ip_toggle));

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
		write(s, buffer, (sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen(data)));
		sleep(1);
	};

	close(s);

};


