#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <errno.h>
#include <string.h>

#include "Checksum.h"

#define SOCKET int

struct arp_pckt {
	struct arphdr arp_hdr;
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

void Print_Ethernet_Address(unsigned char *addr) {

	printf("%02x:%02x:%02x:%02x:%02x:%02x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

};

int main(int argc, char **argv) {

	if (argc < 2) {
		fprintf(stderr, "Usage: Ping IP_Address\n");
		return 1;
	};

	SOCKET s_raw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (s_raw < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	char Packet_Buffer[65535];

	struct ethhdr *eth = (struct ethhdr *) Packet_Buffer;

	struct arp_pckt *arp_hdr = (struct arp_pckt *) (Packet_Buffer + sizeof(struct ethhdr));
	struct arphdr *arp = (struct arphdr *) (Packet_Buffer + sizeof(struct ethhdr));

	struct ifreq ifr;

	strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ);

	if (ioctl(s_raw, SIOCGIFHWADDR, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	memcpy((void *) &(eth->h_source), (void *) (ifr.ifr_hwaddr.sa_data), ETH_ALEN);

	Print_Ethernet_Address(eth->h_source);

	for (int i = 0; i < ETH_ALEN; i++) {
		eth->h_dest[i] = 0xff;
	};

	Print_Ethernet_Address(eth->h_dest);
	eth->h_proto = htons(0x0806);

	arp->ar_hrd = htons(1);
	arp->ar_pro = htons(0x0800);
	arp->ar_hln = ETH_ALEN;
	arp->ar_pln = 4;
	arp->ar_op = htons(1);

	memcpy((void *) &(arp_hdr->ar_sha), (void *) &(eth->h_source), ETH_ALEN);

	if (ioctl(s_raw, SIOCGIFADDR, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	memcpy((void *) &(arp_hdr->ar_sip), (void *) (ifr.ifr_addr.sa_data + 2), 4);

	memcpy((void *) &(arp_hdr->ar_tha), (void *) &(eth->h_dest), ETH_ALEN);

	uint32_t tip = inet_addr(argv[1]);

	memcpy((void *) &(arp_hdr->ar_tip), (void *) &tip, 4);

	struct sockaddr_ll sll;

	if (ioctl(s_raw, SIOCGIFINDEX, &ifr) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	sll.sll_ifindex = ifr.ifr_ifindex;

	sll.sll_protocol = htons(ETH_P_ALL);
	sll.sll_family = AF_PACKET;

	if (bind(s_raw, (struct sockaddr *) &sll, sizeof(sll)) < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	write(s_raw, Packet_Buffer, (sizeof(struct ethhdr) + sizeof(struct arp_pckt)));

	char Recieved_Ethernet_Address[ETH_ALEN];

	while (1) {
		char Recieved_Packet_Buffer[65535];

		read(s_raw, Recieved_Packet_Buffer, 65535);

		struct ethhdr *rcvd_eth = (struct ethhdr *) Recieved_Packet_Buffer;
		struct arp_pckt *rcvd_arp_hdr = (struct arp_pckt *) (Recieved_Packet_Buffer + sizeof(struct ethhdr));
		struct arphdr *rcvd_arp = (struct arphdr *) (Recieved_Packet_Buffer + sizeof(struct ethhdr));

		if ((rcvd_eth->h_proto != htons(0x0806)) || (rcvd_arp->ar_op != htons(2))) {
			continue;
		};

		Print_Ethernet_Address(rcvd_arp_hdr->ar_sha);
		memcpy((void *) &Recieved_Ethernet_Address, (void *) &(rcvd_arp_hdr->ar_sha), ETH_ALEN);
		break;
	};

	memset((Packet_Buffer + sizeof(struct ethhdr)), 0, sizeof(struct arp_pckt));		// Clear the Arp Request Header from the Packet Buffer for the IPv4 Header, and ICMP Header

	memcpy((void *) &(eth->h_dest), (void *) Recieved_Ethernet_Address, ETH_ALEN);		// Overwrite the previous ff:ff:ff:ff:ff:ff with the Ethernet Address of the IP requested
	eth->h_proto = htons(0x0800);

	write(s_raw, Packet_Buffer, sizeof(struct ethhdr));					// Debug the Ethernet Header by sending it over the network to check if the target IP's Ethernet Address is in the h_dest field now

	struct iphdr *ip = (struct iphdr *) (Packet_Buffer + sizeof(struct ethhdr));

	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0;
	ip->id = (getpid() & 0xffff);
	ip->frag_off = 0;
	ip->ttl = 0x64;
	ip->protocol = 1;
	ip->check = 0;
	memcpy((void *) &(ip->saddr), (void *) (ifr.ifr_addr.sa_data + 2), 4);
	memcpy((void *) &(ip->daddr), (void *) &tip, 4);

	struct icmphdr *icmp = (struct icmphdr *) (Packet_Buffer + sizeof(struct ethhdr) + (ip->ihl * 4));

	icmp->type = 8;		// Echo
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->un.echo.id = 0;
	icmp->un.echo.sequence = (getpid() & 0xffff);
	icmp->checksum = Checksum((unsigned char *) icmp, 8);
	ip->tot_len = htons((ip->ihl * 4) + 8);
	ip->check = Checksum((unsigned char *) ip, (ip->ihl * 4));

	write(s_raw, Packet_Buffer, (sizeof(struct ethhdr) + (ip->ihl * 4) + sizeof(struct icmphdr)));

	while (1) {
		unsigned char Recieved_Packet_Buffer[65535];

		read(s_raw, Recieved_Packet_Buffer, 65535);

		struct ethhdr *rcvd_eth = (struct ethhdr *) Recieved_Packet_Buffer;

		struct iphdr *rcvd_ip = (struct iphdr *) (Recieved_Packet_Buffer + sizeof(struct ethhdr));

		struct icmphdr *rcvd_icmp = (struct icmphdr *) (Recieved_Packet_Buffer + sizeof(struct ethhdr) + (ip->ihl * 4));

		if ((rcvd_eth->h_proto != htons(0x0800)) || (rcvd_ip->protocol != 1) || (rcvd_icmp->type != 0) || (rcvd_ip->daddr != ip->saddr)) {
			continue;
		};

		printf("Reply is a success\n");
		break;
	};
	return 0;
};
