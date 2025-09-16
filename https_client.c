#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/crypto.h>
#include <string.h>
#include <ctype.h>


#define SOCKET int

int main(int argc, char *argv[]) {

	if (argc < 3) {
		fprintf(stderr, "Usage: https_client hostname port\n");
		return 1;
	};

	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	SSL_CTX *ssl_tls_context = SSL_CTX_new(TLS_client_method());

	if (!ssl_tls_context) {
		fprintf(stderr, "SSL_CTX_new() Failed\n");
		return 1;
	};

	if (!SSL_CTX_load_verify_locations(ssl_tls_context, "/etc/ssl/certs/ca-certificates.crt", 0)) {
		fprintf(stderr, "SSL_CTX_load_verify_locations() Failed\n");
		ERR_print_errors_fp(stderr);
		return 1;
	};

	char *hostname = argv[1];
	char *port = argv[2];

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));

	struct addrinfo *bind_address;

	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(hostname, port, &hints, &bind_address) < 0) {
		fprintf(stderr, "getaddrinfo() Failed: errno(%d)\n", errno);
		return 1;
	};

	char address[100];
	char service[100];

	if (getnameinfo(bind_address->ai_addr, bind_address->ai_addrlen, address, sizeof(address), service, sizeof(service), NI_NUMERICHOST) < 0) {
		fprintf(stderr, "getnameinfo() Failed: errno(%d)\n", errno);
		return 1;
	};

	printf("Address: %s\nService: %s\n", address, service);

	SOCKET s = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

	if (s < 0) {
		fprintf(stderr, "socket() Failed: errno(%d)\n", errno);
		return 1;
	};

	if (connect(s, bind_address->ai_addr, bind_address->ai_addrlen) < 0) {
		fprintf(stderr, "connect() Failed: errno(%d)\n", errno);
		return 1;
	};

	freeaddrinfo(bind_address);

	SSL *ssl = SSL_new(ssl_tls_context);

	if (!ssl_tls_context) {
		fprintf(stderr, "SSL_new() Failed:\n");
		ERR_print_errors_fp(stderr);
		return 1;
	};

	if (!SSL_set_tlsext_host_name(ssl, hostname)) {
		fprintf(stderr, "SSL_set_tlsext_hostname() Failed\n");
		ERR_print_errors_fp(stderr);
		return 1;
	};

	SSL_set_fd(ssl, s);

	if (SSL_connect(ssl) == -1) {
		fprintf(stderr, "SSL_connect() Failed\n");
		ERR_print_errors_fp(stderr);
		return 1;
	};

	printf("SSL/TLS Cipher Being Used: %s\n", SSL_get_cipher(ssl));

	X509 *cert = SSL_get_peer_certificate(ssl);

	if (!cert) {
		fprintf(stderr, "SSL_get_peer_certificate() Failed\n");
		ERR_print_errors_fp(stderr);
		return 1;
	};

	char *tmp;

	if ((tmp = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0))) {
		printf("Issuer: %s\n", tmp);
		OPENSSL_free(tmp);
	};

	X509_free(cert);

	char buffer[2048];

	sprintf(buffer, "GET / HTTP/1.1\r\n");

	sprintf(buffer + strlen(buffer), "Connection: close\r\n");

	sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);

	sprintf(buffer + strlen(buffer), "User-Agent: https_simple\r\n");

	sprintf(buffer + strlen(buffer), "\r\n");

	SSL_write(ssl, buffer, strlen(buffer));

	printf("Sent Headers:\n%s", buffer);

	memset(&buffer, 0, sizeof(buffer));

	while (1) {
		int bytes_recieved = SSL_read(ssl, buffer, sizeof(buffer));

		if (bytes_recieved < 1) {
			fprintf(stderr, "\nConnection Closed By Host\n");
			break;
		};

		printf("%d of %d bytes recieved\n", strlen(buffer), bytes_recieved);

		printf("%.*s", bytes_recieved, buffer);
	};

	SSL_shutdown(ssl);
	close(s);
	SSL_free(ssl);
	SSL_CTX_free(ssl_tls_context);

	return 0;
};
