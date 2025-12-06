uint16_t tcp_checksum(uint32_t saddr, uint32_t daddr, uint8_t protocol, struct tcphdr *tcp, const char *data, uint16_t data_length) {

	struct pseudoheader {
		uint32_t saddr;
		uint32_t daddr;
		uint8_t zero;
		uint8_t protocol;
		uint16_t tcp_length;
	};

	char pseudo_buffer[65536];

	struct pseudoheader *psdo = (struct pseudoheader *) pseudo_buffer;

	psdo->saddr = saddr;
	psdo->daddr = daddr;
	psdo->zero = 0;
	psdo->protocol = protocol;
	psdo->tcp_length = htons(sizeof(struct tcphdr) + data_length);

	memcpy((void *) (pseudo_buffer + sizeof(struct pseudoheader)), (void *) tcp, sizeof(struct tcphdr));
	memcpy((void *) (pseudo_buffer + sizeof(struct pseudoheader) + sizeof(struct tcphdr)), (void *) data, data_length);

	int16_t result = checksum((void *) pseudo_buffer, (sizeof(struct pseudoheader) + sizeof(struct tcphdr) + data_length));

	return result;

};
