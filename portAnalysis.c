#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
	char *ip = argv[1];
	char command[100];
	int i = 1;
	while (i < 255) {
		sprintf(command, "nmap -p %d %s", i, ip);
		system(command);
		i++;
	};

	return 0;
};

