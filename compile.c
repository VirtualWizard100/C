#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <syscall.h>

int main(int argc, char *argv[]) {
	char command[4096];
	sprintf(command, "nasm -f elf64 %s.s -o %s.o && ld %s.o -o %s", argv[1], argv[1], argv[1], argv[1]);
	printf("%s\n", command);
	system(command);

};
