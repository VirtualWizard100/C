#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/ipv6.h>
#include <string.h>
#include "IP_Header_Struct.h"
#include "Ethernet_Protocols.h"
#include "Protocol.h"

#define SOCKET int

#define RAW 0
#define INFINITE 1
#define DUMP 1
#define ONLY_DUMP 0
#define SHOW_ETH 0
#define DATA 1

int main(int argc, char *argv[]) {

//	struct sockaddr_ll eth;

//	eth.sll_family = AF_PACKET;

	SOCKET s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (s < 0) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", errno);
		return 1;
	};

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));

	unsigned char Ethernet_Address[100];

	strncpy(ifr.ifr_name, "wlan0", 5);
	ifr.ifr_flags = (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_PROMISC | IFF_MULTICAST);

	ioctl(s, SIOCSIFFLAGS, &ifr);
	ioctl(s, SIOCGIFINDEX, &ifr);
	ioctl(s, SIOCGIFHWADDR, &ifr);

	strncpy(Ethernet_Address, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

#if SHOW_ETH == 1
	printf("%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", Ethernet_Address[0], Ethernet_Address[1], Ethernet_Address[2], Ethernet_Address[3], Ethernet_Address[4], Ethernet_Address[5]);
#endif

#define BUFF_SIZE 65536

#if INFINITE == 1
	while (1) {
#else
	if (argc < 2) {
		fprintf(stderr, "Usage: Packet_Sniffer number_of_packets\n");
		return 1;
	};

	uint32_t number_of_packets = atoi(argv[1]);

	for (int i = 0; i < number_of_packets; i++) {
#endif
		char buffer[BUFF_SIZE];

		int bytes_recieved = recvfrom(s, buffer, BUFF_SIZE, 0, NULL, NULL);

		printf("%d bytes recieved\n", bytes_recieved);

		struct ethhdr *Eth = (struct ethhdr *) buffer;

		uint16_t Protocol = ntohs(Eth->h_proto);

/*		if (Protocol == 0x86DD) {
			continue;
		};
*/

               unsigned char *p = (unsigned char *) buffer;

#if DUMP == 1
                for (int i = 0; i <= bytes_recieved; ++i) {

#if RAW == 1
                        printf("%.2x ", *p);
#else
                        printf("%c", *p);
#endif
                        p++;

                        if (i != 0 && i%32 == 0) {
                                printf("\n");
                        };

                };
#endif
                printf("\n\n");

#if DUMP == 1
#if ONLY_DUMP == 1
               printf("\n");

                continue;
#endif
#endif

		printf("Source Ethernet Address: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", Eth->h_source[0], Eth->h_source[1], Eth->h_source[2], Eth->h_source[3], Eth->h_source[4], Eth->h_source[5]);
		printf("Destination Ethernet Address: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n", Eth->h_dest[0], Eth->h_dest[1], Eth->h_dest[2], Eth->h_dest[3], Eth->h_dest[4], Eth->h_dest[5]);
		printf("Protocol: 0x%.4x (%s)\n", Protocol, Ethernet_Protocol(Protocol));

		uint8_t IP_Header_len;
		struct tcphdr *tcp;
		struct udphdr *udp;
		unsigned char *data_p;

		if (Protocol == 0x0800 || Protocol == 0x0806) {
			struct IP_Header *ip = (struct IP_Header *) (buffer + sizeof(struct ethhdr));

			IP_Header_len = (ip->ip_ihl * 4);
			printf("Version: %d\n", ip->ip_version);
			printf("Internet Header Length: %d DWORDs (%d bytes)\n", ip->ip_ihl, IP_Header_len);
			printf("Precedence: %x Throughput: %x Delay: %x Relibility: %x\n", ip->ip_precedence, ip->ip_throughput, ip->ip_delay, ip->ip_relibility);
			printf("Total Length: 0x%.4x (%d)\n", ntohs(ip->ip_total_len), ntohs(ip->ip_total_len));
			printf("Identification: 0x%.4x (%d)\n", ntohs(ip->ip_ident), ntohs(ip->ip_ident));
			printf("Time To Live: 0x%.2x (%d)\n", ip->ip_ttl, ip->ip_ttl);
			printf("Don't Fragment: %b More Fragments: %d\n", ((ntohs(ip->ip_frag_off_res_df_mf) & 0x4000) >> 14), ((ntohs(ip->ip_frag_off_res_df_mf) & 0x2000) >> 13));
			printf("Fragment Offset: 0x%.4x (%d)\n", (ntohs(ip->ip_frag_off_res_df_mf) & 0x1FFF), (ntohs(ip->ip_frag_off_res_df_mf) & 0x1FFF));
			printf("Protocol: %d (%s)\n", ip->ip_proto, get_protocol(ip->ip_proto));
			printf("Checksum: 0x%.4x (%d)\n", ntohs(ip->ip_chksum), ntohs(ip->ip_chksum));
			uint8_t *p_src = (uint8_t *) &(ip->ip_src_addr);
			uint8_t *p_dst = (uint8_t *) &(ip->ip_dst_addr);
			printf("Source Address: %d.%d.%d.%d\n", p_src[0], p_src[1], p_src[2], p_src[3]);
			printf("Destination Address: %d.%d.%d.%d\n", p_dst[0], p_dst[1], p_dst[2], p_dst[3]);

			if (ip->ip_proto == 6) {
				tcp = (struct tcphdr *) (buffer + sizeof(struct ethhdr) + IP_Header_len);

				printf("TCP Header\n\n");

				printf("Source Port: 0x%.4x (%d)\n", ntohs(tcp->source), ntohs(tcp->source));
				printf("Destination Port: 0x%.4x (%d)\n", ntohs(tcp->dest), ntohs(tcp->dest));
				printf("Sequence Number: 0x%.8x (%u)\n", (uint32_t) ntohl(tcp->seq), (uint32_t) ntohl(tcp->seq));
				printf("Acknowledgement Number: 0x%.8x (%u)\n", (uint32_t) ntohl(tcp->ack_seq), (uint32_t) ntohl(tcp->ack_seq));
				printf("Reserved: %d\n", tcp->res1);
				printf("Finished Flag: %b\n", tcp->fin);
				printf("Synchronization Flag: %b\n", tcp->syn);
				printf("Reset Flag: %b\n", tcp->rst);
				printf("Push Flag: %b\n", tcp->psh);
				printf("Acknowledgment Flag: %b\n", tcp->ack);
				printf("Urgent Flag: %b\n", tcp->urg);
#if DATA == 1
				printf("Data\n\n");

				data_p = (unsigned char *) (buffer + sizeof(struct ethhdr) + IP_Header_len + sizeof(struct tcphdr));

				uint32_t Data_len = (bytes_recieved - sizeof(struct ethhdr) - IP_Header_len - sizeof(struct tcphdr));

				for (int i = 0; i <= Data_len; i++) {

#if RAW == 1
					printf("%.2x ", data_p[i]);
#else
					printf("%c", data_p[i]);
#endif
					p++;

					if (i != 0 && i%32 == 0) {
						printf("\n");
					};

				};

				printf("\n");
#endif
			} else if (ip->ip_proto == 17) {
				udp = (struct udphdr *) (buffer + sizeof(struct ethhdr) + IP_Header_len);

				printf("UDP Header\n\n");

				printf("Source Port: 0x%.4x (%d)\n", ntohs(udp->source), ntohs(udp->source));
				printf("Destination Port: 0x%.4x (%d)\n", ntohs(udp->dest), ntohs(udp->dest));
				printf("UDP Length: 0x%.4x (%d)\n", ntohs(udp->len), ntohs(udp->len));
				printf("Checksum: 0x%.4x (%d)\n", ntohs(udp->check), ntohs(udp->check));
#if DATA == 1
				printf("Data\n\n");

				data_p = (unsigned char *) (buffer + sizeof(struct ethhdr) + IP_Header_len + sizeof(struct udphdr));

				uint32_t Data_len = (bytes_recieved - sizeof(struct ethhdr) - IP_Header_len - sizeof(struct udphdr));

				for (int i = 0; i <= Data_len; i++) {

#if RAW == 1
					printf("%.2x ", data_p[i]);
#else
					printf("%c", data_p[i]);
#endif

					if (i != 0 && i%32 == 0) {
						printf("\n");
					};
				};

				printf("\n");
#endif
			};

		} else if (Protocol == 0x86DD) {
			struct ipv6hdr *ipv6 = (struct ipv6hdr *) (buffer + sizeof(struct ethhdr));

			printf("Version: %d\n", ipv6->version);
			printf("Traffic Class: 0x%.2x\n", ipv6->priority);
			uint32_t flow_label = (uint32_t) (((uint32_t) ipv6->flow_lbl[0] << 16) | ((uint32_t) ipv6->flow_lbl[1] << 8) | ((uint32_t) ipv6->flow_lbl[2]));
			printf("Flow Label: 0x%.6x (%d)\n", flow_label, flow_label);
			printf("Payload Length: 0x%.4x (%d)\n", ntohs(ipv6->payload_len), ntohs(ipv6->payload_len));
			printf("Next Header: %d (%s)\n", ipv6->nexthdr, get_protocol(ipv6->nexthdr));
			printf("Hop Limit: %d\n", ipv6->hop_limit);
			struct in6_addr saddr6_p = ipv6->saddr;
			unsigned char *saddr6 = (unsigned char *) &saddr6_p;
			unsigned char saddr6_s[INET6_ADDRSTRLEN];
			if (inet_ntop(AF_INET6, &(saddr6_p), saddr6_s, sizeof(saddr6_s)) < 0) {
				fprintf(stderr, "inet_ntop() Failed\n");
				return 1;
			};
			printf("Source Address: %s\n", saddr6_s);
			struct in6_addr daddr6_p = ipv6->daddr;
			unsigned char daddr6_s[INET6_ADDRSTRLEN];
			if (inet_ntop(AF_INET6, &daddr6_s, (char *) daddr6_s, sizeof(daddr6_s)) == NULL) {
				fprintf(stderr, "inet_ntop() Failed\n");
				return 1;
			};
			printf("Destination Address: %s\n", daddr6_s);

			if (ipv6->nexthdr == 6) {
				tcp = (struct tcphdr *) (buffer + sizeof(struct ethhdr) + sizeof(struct ipv6hdr));

				printf("TCP Header\n\n");

				printf("Source Port: 0x%.4x (%d)\n", ntohs(tcp->source), ntohs(tcp->source));
                printf("Destination Port: 0x%.4x (%d)\n", ntohs(tcp->dest), ntohs(tcp->dest));
                printf("Sequence Number: 0x%.8x (%u)\n", (uint32_t) ntohl(tcp->seq), (uint32_t) ntohl(tcp->seq));
                printf("Acknowledgement Number: 0x%.8x (%u)\n", (uint32_t) ntohl(tcp->ack_seq), (uint32_t) ntohl(tcp->ack_seq));
                printf("Reserved: %d\n", tcp->res1);
                printf("Finished Flag: %b\n", tcp->fin);
                printf("Synchronization Flag: %b\n", tcp->syn);
                printf("Reset Flag: %b\n", tcp->rst);
                printf("Push Flag: %b\n", tcp->psh);
                printf("Acknowledgment Flag: %b\n", tcp->ack);
                printf("Urgent Flag: %b\n", tcp->urg);

#if DATA == 1
				data_p = (unsigned char *) (buffer + sizeof(struct ethhdr) + sizeof(struct ipv6hdr) + sizeof(struct tcphdr));

				uint32_t Data_len = (uint32_t) (bytes_recieved - sizeof(struct ethhdr) - sizeof(struct ipv6hdr) - sizeof(struct tcphdr));

				printf("Data\n\n");

				for (int i = 0; i <= Data_len; i++) {

#if RAW == 1
					printf("%.2x ", data_p[i]);
#else
					printf("%c", data_p[i]);
#endif

					if (i != 0 && i%32 == 0) {
						printf("\n");
					};

					p++;
				};

				printf("\n");
#endif
			} else if (ipv6->nexthdr == 17) {
				printf("UDP Header\n\n");

				udp = (struct udphdr *) (buffer + sizeof(struct ethhdr) + sizeof(struct ipv6hdr));

				printf("Source Port: 0x%.4x (%u)\n", ntohs(udp->source), ntohs(udp->source));
				printf("Destination Port: 0x%.4x (%u)\n", ntohs(udp->dest), ntohs(udp->dest));
				printf("UDP Length: 0x%.4x (%u)\n", ntohs(udp->len), ntohs(udp->len));
				printf("Checksum: 0x%.4x (%u)\n", ntohs(udp->check), ntohs(udp->check));

#if DATA == 1
				data_p = (unsigned char *) (buffer + sizeof(struct ethhdr) + sizeof(struct ipv6hdr) + sizeof(struct udphdr));

				uint32_t Data_len = (uint32_t) (bytes_recieved - sizeof(struct ethhdr) - sizeof(struct ipv6hdr) - sizeof(struct udphdr));

				printf("Data\n\n");

				for (int i = 0; i <= Data_len; i++) {
#if RAW == 1
					printf("%.2x ", data_p[i]);
#else
					printf("%c", data_p[i]);
#endif
					p++;

					if (i != 0 && i%32 == 0) {
						printf("\n");
					};

				};

				printf("\n");
#endif
			};

		};

		printf("\n");
		printf("-----------------------------------------------------------------------------------------------\n");
		printf("\n");

	};

	return 0;

};
