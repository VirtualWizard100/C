uint16_t Checksum(unsigned char *ip, uint32_t iphdr_len) {


	long Checksum = 0;	//or memset(&Checksum, 0, sizeof(Checksum));

	uint16_t *p = (uint16_t *) ip;	//16 bit pointer to struct iphdr *ip cast as unsigned char *ip, meaning p is in the pointer mode

	while (iphdr_len > 1) {		//while iphdr_len is atleast equal to 2 or more

		Checksum += *p;		//Checksum += dereferenced *p

		if (Checksum > 0x80000000) {		//if high order bit is set
			Checksum = (Checksum & 0xFFFF) + (Checksum >> 16);	//Checksum = (Checksum & 0xFFFF) + ((Checksum & 0xFFFF0000) >> 16)
		};
		p++;		//move p foreward by 2 bytes
		iphdr_len -= 2;
	};

	if (iphdr_len) {	//if iphdr_len > 0
		Checksum += (unsigned char) (*p & 0xFF); //Checksum = (*p & 0xFF) cast as uint16_t, dereferenced
	};

	while (Checksum >> 16) { //While (Checksum >> 16) != 0
		Checksum = (Checksum & 0xFFFF) + (Checksum >> 16);
	};

	return ~Checksum;	//return one's compliment version of Checksum
};
