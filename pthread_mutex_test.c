#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define SOCKET int

pthread_mutex_t mutex;
static unsigned long sum = 0;
void *long_function(void *message) {

	pthread_mutex_lock(&mutex);

	for (int i = 0; i < 100000000; i++) {
		*(unsigned long *) message += 1;
	};

	pthread_mutex_unlock(&mutex);

	sum += *(unsigned long *) message;

	return message;
};

int main(int argc, char **argv) {

	pthread_t thread1;
	pthread_t thread2;

	pthread_mutex_init(&mutex, NULL);

	unsigned long message = (0x1UL << 24);

	unsigned long message2 = (0x1UL << 17);

	pthread_create(&thread1, NULL, long_function, &message);
	pthread_create(&thread2, NULL, long_function, &message2);

	printf("Ahoy\n");

	void *pthread_result;
	void *pthread_result2;

	pthread_join(thread1, &pthread_result);
	pthread_join(thread2, &pthread_result2);

	printf("0x%08lx (%lu)\n", *(unsigned long *) pthread_result, *(unsigned long *) pthread_result);
	printf("0x%08lx (%lu)\n", *(unsigned long *) pthread_result2, *(unsigned long *) pthread_result2);
	printf("0x%08lx (%lu)\n", sum, sum);

	pthread_mutex_destroy(&mutex);

	return 0;

};
