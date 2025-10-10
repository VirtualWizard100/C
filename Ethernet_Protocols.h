#include <asm/byteorder.h>

char *Ethernet_Protocol(uint16_t proto) {
	if (proto == 0x0800) {
		return "IPv4";
	} else if (proto == 0x0806) {
		return "ARP";
	} else if (proto == 0x86dd) {
		return "IPv6";
	} else {
		return "Unknown";
	};
};
