#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include "IP_Header_Struct.h"

#define SOCKET int
#define MY_IP 0
#define HEX 0
//IP datagrams that are fragmented need to make the first fragment 8 bytes

int main() {

	struct sockaddr_ll phys_address;
	memset(&phys_address, 0, sizeof(phys_address));

	phys_address.sll_family = AF_PACKET;
	phys_address.sll_ifindex = if_nametoindex("wlan0");
	phys_address.sll_protocol = htons(ETH_P_IP);

	unsigned char Ethernet_Address[] = { 0x9c, 0x30, 0x5b, 0x1f, 0x2f, 0x95 };

	memset(&(phys_address.sll_addr), 0, sizeof(phys_address.sll_addr));
#if MY_IP == 1
	phys_address.sll_addr[0] = Ethernet_Address[0];
	phys_address.sll_addr[1] = Ethernet_Address[1];
	phys_address.sll_addr[2] = Ethernet_Address[2];
	phys_address.sll_addr[3] = Ethernet_Address[3];
	phys_address.sll_addr[4] = Ethernet_Address[4];
	phys_address.sll_addr[5] = Ethernet_Address[5];
#endif
	phys_address.sll_halen = ETH_ALEN;

	printf("sll_family: %d\n", AF_PACKET);
	printf("Interface wlan0 has index %d\n", phys_address.sll_ifindex);

	SOCKET s = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));	//SOCK_RAW to see raw packets including the link level header, SOCK_DGRAM to see raw packets, and strip the link level header

	if (s < 0) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", errno);
		return 1;
	};

//	struct ifreq interface;

//	char *device = "wlan0";

//	strncpy((char *) interface.ifr_name, device, IFNAMSIZ);

	if (bind(s, (struct sockaddr *) &phys_address, sizeof(phys_address)) < 0) {
		fprintf(stderr, "bind() Failed: errno(%d)\n", errno);
		return 1;
	};

//	ioctl(s, SIOCGIFINDEX, &interface);

	unsigned char IP_Header_Buffer[65536];

	struct IP_Header *peer_address = (struct IP_Header *) &IP_Header_Buffer;

	socklen_t addr_len = sizeof(peer_address);

	while (1) {

		int bytes_recieved = recvfrom(s, IP_Header_Buffer, sizeof(IP_Header_Buffer), 0, NULL, NULL);

		unsigned char *p_packet = IP_Header_Buffer;

		printf("Recieved %d bytes\n",  bytes_recieved);
#if HEX == 1
		for (int i = 0; i <= bytes_recieved; i++) {
			printf("%.2x ", *p_packet);
			p_packet++;

			if (i%32 == 0) {
				printf("\n");
			};
		};
#else
		for (int i = 0; i <= bytes_recieved; i++) {
                        printf("%c ", *p_packet);
                        p_packet++;

                        if (i%32 == 0) {
                                printf("\n");
                        };
                };
#endif
		printf("\n");

		uint32_t src_address = peer_address->ip_src_addr;

		uint8_t *p = (uint8_t *) &src_address;

		if (p[0] > 200) {
			src_address = htonl(src_address);
		};

		printf("Source Address: %d.%d.%d.%d\n", p[0], p[1], p[2], p[3]);

		uint32_t dst_address = peer_address->ip_dst_addr;

		uint8_t *p2 = (uint8_t *) &dst_address;

		printf("Destination Address: %d.%d.%d.%d\n", p2[0], p2[1], p2[2], p2[3]);

		printf("Version: %x\n", peer_address->ip_version);

		printf("Internet Header Length: %x\n", peer_address->ip_ihl);

		printf("Precedence: %x: Delay: %x: Throughput: %x: Relibility: %x\n", peer_address->ip_precedence, peer_address->ip_delay, peer_address->ip_throughput, peer_address->ip_relibility);

//		printf("Reserved: %d\n", peer_address->ip_reserved);

//		printf("Fragment Offset: %d\n", peer_address->ip_frag_off);

		printf("Total Length: %d\n", ntohs(peer_address->ip_total_len));

		printf("Identification: %d\n", ntohs(peer_address->ip_ident));
		printf("TTL: %d\n", peer_address->ip_ttl);
		printf("Checksum: %d\n", ntohs(peer_address->ip_chksum));
		printf("Protocol: %d\n", peer_address->ip_proto);
		printf("Fragment Offset: %x\n", (ntohs(peer_address->ip_frag_off_res_df_mf) & 0x1FFF));
		printf("Reserved: %b\n", ((ntohs(peer_address->ip_frag_off_res_df_mf) & 0x8000) >> 15));
		printf("Don't Fragment: %b\n", ((ntohs(peer_address->ip_frag_off_res_df_mf) & 0x4000) >> 14));
		printf("More Fragments: %b\n", ((ntohs(peer_address->ip_frag_off_res_df_mf) & 0x2000) >> 13));
//		printf("Options: %d\n", peer_address->ip_options);
/*		printf("Fragment Offset: %d\n", peer_address->ip_frag_off);
		printf("Don't Fragment: %d\n", peer_address->ip_df);
		printf("More Fragments: %d\n", peer_address->ip_mf);
*/
		printf("\n");
	};

	return 0;

};


