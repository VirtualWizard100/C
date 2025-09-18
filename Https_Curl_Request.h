void send_curl(struct client_info *client) {
	char *scurl = "HTTP/1.1 400 Bad Request\r\n" "Connection: close\r\n" "Content-Type: text/plain\r\n" "Content-Length: 11\r\n\r\nNo Curls XD";

	SSL_write(client->ssl, scurl, strlen(scurl));

	drop_client(client);
};
