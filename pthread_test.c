#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define SOCKET int

void *long_function(void *message) {

	for (int i = 0; i < 100000000; i++) {
		*(unsigned long *) message += 1;
	};

	return message;
};

int main(int argc, char **argv) {

	pthread_t thread1;

	unsigned long message = (0x1UL << 24);

	pthread_create(&thread1, NULL, long_function, &message);

	printf("Ahoy\n");

	void *pthread_result;

	pthread_join(thread1, &pthread_result);

	pthread_kill(thread1, SIGTERM);

	printf("0x%08x\n", *(unsigned long *) pthread_result);

	return 0;

};
