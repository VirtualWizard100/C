struct pseudoheader {
	uint32_t src_addr;
	uint32_t dst_addr;
	uint8_t zero;
	uint8_t protocol;
	uint16_t tcp_length;
};

uint16_t tcp_checksum(uint8_t *tcp_hdr, uint16_t length, struct iphdr *ip_hdr) {

	uint32_t sum = 0;			// Make a 32 bit variable to store the values of all the 16 bit words in, and initialize it to 0

	uint16_t *bfr_ptr = (uint16_t *) malloc(sizeof(struct pseudoheader) + length);		// Allocate the amount of bytes needed for the whole header, and point to it with a 16 bit pointer

	struct pseudoheader *psdohdr = (struct pseudoheader *) bfr_ptr;

	psdohdr->src_addr = ip_hdr->saddr;
	psdohdr->dst_addr = ip_hdr->daddr;
	psdohdr->zero = 0;
	psdohdr->protocol = 6;
	psdohdr->tcp_length = htons(length);									// Needs to be in network byte order to calculate the checksum

	memcpy((void *) ((uint8_t *) bfr_ptr + sizeof(struct pseudoheader)), (void *) tcp_hdr, length);		// Allocate the TCP Header and data at the offset of the pseudoheader

	length += sizeof(struct pseudoheader);					// Add the size of the pseudoheader in bytes to length for the total length of the buffer

	while (length > 1) {
		sum += *bfr_ptr++;
		length -= 2;
	};

	if (length > 0) {
		sum += (uint16_t) (*(uint8_t *) bfr_ptr) << 8;		// Change the bfr_ptr pointer type to a uint8_t pointer to point to the high order byte, dereference it to get the 8 bit value shift it to the right by 8, and cast it as a uint16_t to put the byte in the high order byte of the uint16_t value
	};

	while (sum >> 16) {						// while there is still a value in the high order 16 bits of sum
		sum = (sum & 0xffff) + (sum >> 16);
	};

	return (uint16_t)~sum;						// RFC 9293 the TCP Checksum is the 16 bit "one's compliment" of the "one's compliment sum" of all 16 bit words of the header, and text, essentially ~~, which can be (uint16_t)~

};
