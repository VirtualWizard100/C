#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <netdb.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/if_arp.h>
#include <errno.h>
#include <string.h>

#include "Checksum.h"
#include "tcp_checksum.h"

#define SOCKET int

struct arp_pckt {
	struct arphdr *arp_hdr;
	unsigned char ar_sha[ETH_ALEN];
	unsigned char ar_sip[4];
	unsigned char ar_tha[ETH_ALEN];
	unsigned char ar_tip[4];
};

char *get_errno_message() {
	char *errno_message = strerror(errno);

	static char buffer[100];

	sprintf(buffer, "errno(%d): %s", errno, errno_message);

	return buffer;
};

void Print_Ethernet_Address(unsigned char bfr[ETH_ALEN]) {

	printf("%02x:%02x:%02x:%02x:%02x:%02x\n", bfr[0], bfr[1], bfr[2], bfr[3], bfr[4], bfr[5]);

};

int main(int argc, char **argv) {

	if (argc < 3) {
		fprintf(stderr, "Usage: tcp_pckt_injctn IP_Address/domain message\n");
		return 1;
	};

	SOCKET s_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (s_raw < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	struct ifreq ifr;

	strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);

	if (ioctl(s_raw, SIOCGIFINDEX, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	struct sockaddr_ll sll;

	sll.sll_family = AF_PACKET;
	sll.sll_protocol = htons(ETH_P_ALL);
	sll.sll_ifindex = ifr.ifr_ifindex;

	if (bind(s_raw, (struct sockaddr *) &sll, sizeof(sll)) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	char Packet_Buffer[65535];

	struct ethhdr *eth = (struct ethhdr *) Packet_Buffer;

	if (ioctl(s_raw, SIOCGIFHWADDR, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	memcpy((void *) &(eth->h_source), (void *) (ifr.ifr_hwaddr.sa_data), ETH_ALEN);

	for (int i = 0; i < ETH_ALEN; i++) {
		eth->h_dest[i] = 0xff;
	};

	eth->h_proto = htons(0x0806);

	struct arp_pckt *arp_hdr = (struct arp_pckt *) (Packet_Buffer + sizeof(struct ethhdr));
	struct arphdr *arp = (struct arphdr *) (Packet_Buffer + sizeof(struct ethhdr));

	arp->ar_hrd = htons(1);
	arp->ar_pro = htons(0x0800);
	arp->ar_hln = ETH_ALEN;
	arp->ar_pln = 4;
	arp->ar_op = htons(1);

	memcpy((void *) (arp_hdr->ar_sha), (void *) (ifr.ifr_hwaddr.sa_data), ETH_ALEN);

	if (ioctl(s_raw, SIOCGIFADDR, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	memcpy((void *) (arp_hdr->ar_sip), (void *) (ifr.ifr_addr.sa_data + 2), 4);

	for (int i = 0; i < ETH_ALEN; i++) {
		arp_hdr->ar_tha[i] = 0xff;
	};

	uint32_t daddr = inet_addr(argv[1]);

	memcpy((void *) &(arp_hdr->ar_tip), (void *) &daddr, 4);

	write(s_raw, Packet_Buffer, (sizeof(struct ethhdr) + sizeof(struct arp_pckt)));

	unsigned char Ethernet_Address[ETH_ALEN];

	while (1) {
		char rcvd_pckt_bfr[65535];

		read(s_raw, rcvd_pckt_bfr, 65535);

		struct ethhdr *rcvd_eth = (struct ethhdr *) rcvd_pckt_bfr;

		struct arp_pckt *rcvd_arp_hdr = (struct arp_pckt *) (rcvd_pckt_bfr + sizeof(struct ethhdr));

		struct arphdr *rcvd_arp = (struct arphdr *) (rcvd_pckt_bfr + sizeof(struct ethhdr));

		if ((rcvd_eth->h_proto != htons(0x0806)) || (rcvd_arp->ar_op != htons(2))) {
			continue;
		};

		memcpy((void *) Ethernet_Address, (void *) (rcvd_arp_hdr->ar_sha), ETH_ALEN);

		Print_Ethernet_Address(rcvd_arp_hdr->ar_sha);

		break;

	};

	memset((Packet_Buffer + sizeof(struct ethhdr)), 0, sizeof(struct arp_pckt));

	memcpy((void *) (eth->h_dest), (void *) Ethernet_Address, ETH_ALEN);

	eth->h_proto = htons(0x0800);

	struct iphdr *ip = (struct iphdr *) (Packet_Buffer + sizeof(struct ethhdr));

	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0;
	ip->id = getpid();
	ip->ttl = 0x40;
	ip->protocol = 6;
	ip->check = 0;
	memcpy((void *) &(ip->saddr), (void *) (ifr.ifr_addr.sa_data + 2), 4);
	memcpy((void *) &(ip->daddr), (void *) &daddr, 4);

	//ip->tot_len and ip->check last

	struct tcphdr *tcp = (struct tcphdr *) (Packet_Buffer + sizeof(struct ethhdr) + (ip->ihl * 4));

	tcp->source = htons(getpid());
	tcp->dest = htons(80);
	tcp->seq = getpid();
	tcp->ack_seq = 0;
	tcp->doff = (sizeof(struct tcphdr)/4);
	tcp->syn = 1;
	tcp->window = htons(1460);
	tcp->check = 0;
	tcp->urg_ptr = 0;

	tcp->check = tcp_checksum((unsigned char *) tcp, sizeof(struct tcphdr), ip);

	ip->tot_len = htons((ip->ihl * 4) + sizeof(struct tcphdr));
	ip->check = Checksum((unsigned char *) ip, (ip->ihl * 4));

	write(s_raw, Packet_Buffer, (sizeof(struct ethhdr) + (ip->ihl * 4) + sizeof(struct tcphdr)));

	return 0;

};

