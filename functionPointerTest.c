#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

void led_on() {
	printf("LED is on\n");
	exit(0);
};

void led_off() {
	printf("LED is off\n");
	exit(0);
};

void led_reset() {
	printf("LED has been reset\n");
	exit(0);
};

struct command_function {
        char *command;
        void (*function)(void);
};

static struct command_function command_list[] = {{"ON", led_on}, {"OFF", led_off}, {"RESET", led_reset}};

int main(int argc, char **argv) {

	if (argc < 2) {
		fprintf(stderr, "Usage: functionPointerTest command\n");
		return 1;
	};

//	printf("%d %d %d\n", sizeof(command_list), sizeof(struct command_function), (sizeof(command_list) / sizeof(struct command_function)));

	char command[strlen(argv[1])];

	for (int i = 0; i < strlen(argv[1]); i++) {
		command[i] = toupper(argv[1][i]);
	};

	for (int i = 0; i < (sizeof(command_list) / sizeof(struct command_function)); i++) {
		if (strcmp(command, command_list[i].command) == 0) {
			command_list[i].function();
			break;
		};
	};

	printf("Unknown command %s\n", argv[1]);

	return 1;

};
