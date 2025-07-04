#include "chap05.h"

const unsigned char *print_name(const unsigned char *msg, const unsigned char *p, const unsigned char *end) {	//msg is a pointer to the beginning of the message, p is a pointer to the current name to print, and end is a pointer to one past the end of the message

	if (p + 2 > end) {							//if the address of p + 2 is greater than the address of end, then there is no text in the p pointer, so there is no name
		fprintf(stderr, "End of Message\n");
		exit(1);
	};

	if ((*p & 0xC0) == 0xC0) { 						//If a length has its two highest bits set (that is, 0xc0), then it and the next byte should be interpreted as a pointer instead.
		const int k = ((*p & 0x3F) << 8) + p[1];			//Store the value of p[0] & 0x3f << 8 = + p[1] = 12 bit pointer into k, (p[0] & 0b00111111) << 8 + p[1]
		p += 2; 							//The address of p = The Address of p moved over 2 bytes
		printf(" (pointer (%d)) ", k);					//The 12 bit pointer that we just stored in k
		print_name(msg, msg+k, end); 					//Call print_name again Accept the beginning message pointer is offset k pointer bytes, then stored in p to get to the QNAME beginning to read the hostname again (sometimes the answer puts a pointer to the hostname in the question header in NAME to not repeat the same name twice)
		return p;
	} else {
		const int len = *p++;						//The value of the length byte + 1 to get to the next length byte at the end

		if (p + len + 1 > end) {					//If the address of the current label + len + 1 is greater than the end address
			fprintf(stderr, "End of Message\n");
			exit(1);
		};

		printf("%.*s", len, p);						//print length bytes for the current label

		p += len; 							//p pointer is moved the amount of bytes foreward of the length of the current name to go to the next name

		if (*p) { 							//If the length byte in p isn't 0 (the end)
			printf(".");
			return print_name(msg, p, end); 			//Call print_name again with the new label
		} else {
			return p+1; 						//The end of the message, onto the TYPE of the Header
		};
	};
};



void *print_dns_message(const char *message, int msg_length) {			//message = the whole dns message, msg_length = the length of the whole dns message in bytes

	if (msg_length < 12) {
		fprintf(stderr, "Message is too short to be valid\n");
		exit(1);
	};

	unsigned char *msg = (unsigned char *) message;

#ifdef PRINT_RAW_MESSAGE
	int i;
	for (i = 0; i < msg_length; ++i) {
		unsigned char r = msg[i];

		printf("%02d: %02X %03d '%c'\n", i, r, r, r);
	};
#endif

	//ID is the first 16 bits of the dns message


	printf("Message ID: %x%x\n", msg[0], msg[1]); //The ID of the dns message



	// [QR(1)] [OPCODE(4)] [AA(1)] [TC(1)] [RD(1)] = next 8 bits (byte)

	const int QR = (msg[2] & 0x80) >> 7;	//AND msg[2] and 0x80 to only retrieve the bit for QR, then shift it right by 7 to put it as the least signifigant byte

	printf("QR = %d %s\n", QR, QR ? "response" : "query");

	const int OPCODE = (msg[2] & 0x78) >> 3;	//msg[2] & 0b01111000 >> 3 for the OPCODE

	switch (OPCODE) {
		case 0:
			printf("OPCODE: standard\n");
			break;
		case 1:
			printf("OPCODE: reverse\n");
			break;
		case 2:
			printf("OPCODE: status\n");
			break;
		default:
			printf("OPCODE: reserved\n");
	};

	const int AA = (msg[2] & 0x04) >> 2;		//Authoritative Answer, 1 for yes, 0 for no, msg[2] & 0b00000100 >> 2

	printf("AA = %d %s\n", AA, AA ? "Authoritative" : "");

	const int TC = (msg[2] & 0x02) >> 1;		//Truncation, 1 for message was truncated, 0, for not, msg[2] & 0b00000010 >> 1

	printf("TC = %d %s\n", TC, TC ? "Message truncated" : "");

	const int RD = (msg[2] & 0x01);			//Recursion Desired, 1 for yes, 0 for no, msg[2] & 0b00000001

	printf("RD = %d %s\n", RD, RD ? "Recursion Desired" : "");


	//[RA(1)] [Z(3)] [RCODE(4)] = next 8 bits (byte)

	if (QR) { //If QR is a response

		const int RCODE = (msg[3] & 0x07);	//Response Code, the values are as follows

		printf("RCODE = %d\n", RCODE);

		switch (RCODE) {

			case 0:
				printf("RCODE: Success\n");
				break;
			case 1:
				printf("RCODE: Format Error\n");
				break;
			case 2:
				printf("RCODE: Server Failure\n");
				break;
			case 3:
				printf("RCODE: Name Error\n");
				break;
			case 4:
				printf("RCODE: Not Implemented\n");
				break;
			case 5:
				printf("RCODE: Refused\n");
				break;
			default:
				printf("RCODE: ?\n");

		};

		if (RCODE != 0) {
			return 0;
		};

	};


	//[QDCOUNT(2 bytes)] [ANCOUNT(2 bytes)] [NSCOUNT(2 bytes)] [ARCOUNT(2 bytes)] = 8 bytes

	const int QDCOUNT = (msg[4] << 8) + msg[5];		//The number of Questions in the Question Section
	const int ANCOUNT = (msg[6] << 8) + msg[7];		//The number of Answers in the Answer Section
	const int NSCOUNT = (msg[8] << 8) + msg[9];		//The number of Name Server Records in the Authority Records Section
	const int ARCOUNT = (msg[10] << 8) + msg[11];		//The number of Resource Records in the Resource Records Section

	printf("QDCOUNT = %d\n", QDCOUNT);
	printf("ANCOUNT = %d\n", ANCOUNT);
	printf("NSCOUNT = %d\n", NSCOUNT);
	printf("ARCOUNT = %d\n", ARCOUNT);


	//Question Format, [NAME(16)] [QTYPE(16)] [QCLASS(16)] = 48 bits (6 bytes)

	const unsigned char *p = msg + 12; //Start of the message + 12 bytes to accomodate for the Dns Header

	const unsigned char *end = msg + msg_length; //The Address of the beginning of the message + the message length will get you the end address of the message


	if (QDCOUNT) { //If QDCOUNT isn't 0

		int i;
		for (i = 0; i < QDCOUNT; ++i) {	//i < amount of questions, if i = QDCOUNT, end of loop
			if (p >= end) { //If there are no valid questions
				fprintf(stderr, "End of Message\n");
				exit(1);
			};

			printf("Query %2d\n", i + 1);

			printf(" name: ");

			p = print_name(msg, p, end);		//Goes through print_name(), and iterates through each question one at a time, and prints the question names out until the end, then returns the end of message (p+1),  in which case the next if statement will be true
			printf("\n");

			if (p + 4 > end) {
				fprintf(stderr, "End of Message\n");
				exit(1);
			};

			const int QTYPE = (p[0] << 8) + p[1];	//16 bits (2 bytes) after the question name is the Question Type

			printf(" QTYPE: %d\n", QTYPE);

			p += 2;

			const int QCLASS = (p[0] << 8) + p[1];	//16 bits (2 bytes) after the Question Type is the Question Class, which is always set to 1 for the internet

			printf(" QCLASS: %d\n", QCLASS);

			printf("\n");

			p += 2;
		};

	};

	if (ANCOUNT || NSCOUNT || ARCOUNT) {

		int i;

		for (i = 0; i < ANCOUNT + NSCOUNT + ARCOUNT; ++i) {

			if (p >= end) {		//If the name pointer is already at the end of the message
				fprintf(stderr, "End of Message\n");
				exit(1);
			};

			printf("Answer: %2d\n", i + 1);
			printf(" NAME: ");

			p = print_name(msg, p, end);		//Else print all names
			printf("\n");				//Followed by newline

			if (p + 10 > end) {
				fprintf(stderr, "End of Message\n");
				exit(1);
			};

			const int TYPE = (p[0] << 8) + p[1];	//The first 2 bytes after the Answer Name is TYPE

			printf(" TYPE %d\n", TYPE);

			p += 2;

			const int ACLASS = (p[0] << 8) + p[1]; //The 2 bytes after the TYPE is ACLASS

			printf(" CLASS: %d\n", ACLASS);

			p += 2;

			const int TTL = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];	//Time to live, how many seconds can the current record(s) stay in the cache

			printf(" TTL: %u\n", TTL);

			p += 4;

			const int RDLENGTH = (p[0] << 8) + p[1];

			if (p + RDLENGTH + 1 > end) {
				fprintf(stderr, "End of Message\n");
				exit(1);
			};

			printf(" RDLENGTH: %d\n", RDLENGTH);

			p += 2;

			//RDATA Section

			if (RDLENGTH == 4 && TYPE == 1) {	//A Record
				printf(" Address: ");
				printf("%d.%d.%d.%d\n\n", p[0], p[1], p[2], p[3]);	//IPv4 Address
			};

			if (TYPE == 15 && RDLENGTH > 3) {	//MX Record
				const int Preference = (p[0] << 8) + p[1];
				printf("Preference: %d\n", Preference);
				printf("MX: ");
				print_name(msg, p+2, end);
				printf("\n");
			} else if (RDLENGTH == 16 && TYPE == 28) {	//AAAA Record

				printf(" Address: ");

				int j;
				for (j = 0; j < RDLENGTH; j+=2) {

					printf("%02x%02x", p[j], p[j+1]);

					if (j + 2 < RDLENGTH) {
						printf(":");
					};

				};
				printf("\n\n");
			} else if (TYPE == 16) {		//TXT Record

				printf("TXT: %.*s\n", RDLENGTH -1, p+1);
			} else if (TYPE == 5) {			//CNAME Record

				printf("CNAME: ");
				print_name(msg, p, end);
				printf("\n");
			};

			p += RDLENGTH;

		};	//For
	};	//If

	if (p != end) {
		printf("There is some unread data left over\n");
	};

	printf("\n");

};

int main(int argc, char *argv[]) {

	if (argc < 3) {
		printf("Usage:\n\tdns_query hostname type\n");
		printf("Example:\n\tdns_query example.com aaaa\n");
		exit(0);
	};

	if (strlen(argv[1]) > 255) {
		fprintf(stderr, "Hostname too long\n");
		exit(1);
	};

	unsigned char TYPE;

	if (strcmp(argv[2], "a") == 0) {
		TYPE = 1;
	} else if (strcmp(argv[2], "aaaa") == 0) {
		TYPE = 28;
	} else if(strcmp(argv[2], "mx") == 0) {
		TYPE = 15;
	} else if (strcmp(argv[2], "txt") == 0) {
		TYPE = 16;
	} else if (strcmp(argv[2], "any") == 0) {
		TYPE = 255;
	} else {
		fprintf(stderr, "Unknown Type: %s, Use a, aaaa, mx, txt, or any\n", argv[2]);
		exit(1);
	};


	struct addrinfo hints;
	struct addrinfo *bind_address;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo("8.8.8.8", "53", &hints, &bind_address) < 0) {
		fprintf(stderr, "getaddrinfo() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	char address_buffer[100];
	char service_buffer[100];

	if (getnameinfo(bind_address->ai_addr, bind_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST | NI_NUMERICSERV) < 0) {
		fprintf(stderr, "getnameinfo() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	printf("Dns Server Address: %s\nDns Port: %s\n", address_buffer, service_buffer);

	SOCKET peer_socket = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

	if (!ISVALIDSOCKET(peer_socket)) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	char query_dns_header[1024] = {
				0xAB, 0xCD,	//ID
				0x01, 0x00,	//[QR(1)] [OPCODE(4)] [AA(1)] [TC(1)] [RD(1)] [RA(1)] [Z(3)] [RCODE(4)] (We're setting Recursion Desired (RD) to 1 for yes)
				0x00, 0x01,	//QDCOUNT
				0x00, 0x00,	//ANCOUNT
				0x00, 0x00,	//NSCOUNT
				0x00, 0x00	//ARCOUNT
			   };

	char *p = query_dns_header + 12;	//Pointer to start of Question Header to append hostname bytes by byte (QNAME)

	char *h = argv[1];	//Dns Server Name

	while(*h) {	//Loops through each label in the hostname until *h = 0, then ends the loop
		char *len = p;	//The Beginning of the current label
		p++;

		if (h != argv[1]) {	//If *h is not at the start position of argv[1]
			++h;
		};

		while (*h && *h != '.') {
			*p++ = *h++;	//Dereferenced char byte pointer p is equal to dereferenced char byte pointer h, then both move foreward a position after (putting non . character into QNAME) until *h == '.', or *h == 0
		};
		*len = p - len -1;	//address of p after while loop - beginning address of len - 1 = all bytes in name but end byte from last ++ for current label
	};

	*p++ = 0;

	printf("%s\n", query_dns_header+12);

	*p++ = 0x00;
	*p++ = TYPE; //[QTYPE(16)]

	*p++ = 0x00;
	*p++ = 0x01; //[QCLASS(16)]

	const int query_size = p - query_dns_header;

	int bytes_sent = sendto(peer_socket, query_dns_header, query_size, 0, bind_address->ai_addr, bind_address->ai_addrlen);

	if (bytes_sent < 0) {
		fprintf(stderr, "sendto() Failed: errno(%d)\n", GETERRNO());
		return 1;
	};

	printf("%d bytes sent\n", bytes_sent);

	print_dns_message(query_dns_header, query_size);

	char read[1024];

	int bytes_recieved = recvfrom(peer_socket, read, 1024, 0, 0, 0);

	printf("%d bytes recieved\n", bytes_recieved);

	print_dns_message(read, bytes_recieved);

	printf("\n");

	CLOSESOCKET(peer_socket);

	return 0;

};
