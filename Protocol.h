char *get_protocol(int protocol) {
	switch (protocol) {
		case 1:
			return "ICMP";
		case 6:
			return "TCP";
		case 17:
			return "UDP";
		default:
			return "Unknown";
	};
};
