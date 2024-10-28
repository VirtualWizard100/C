#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

float squareroot(float *num) {
	float a;
	printf("%f\n", *num);
	while (1) {
		a += .01;
		if ((a*a) == *num) {
			break;
		} else {
			continue;
		};
	break;
	};
	return a;
};

struct Person {
	char name[50];
	int age;
	union {
		float f_height;
		int i_height;
	} height;
};
typedef struct Person Test;

int main(int argc, char *argv[]) {
	struct Person Asher;
	strcpy(Asher.name, argv[1]);
	Asher.age = 22;
	float Asherfheight = Asher.height.f_height;
	Asherfheight = 5.9;
	printf("%s\n%d\n%f\n", Asher.name, Asher.age, Asherfheight);
	printf("%d\n", argc);
	float num = atof(argv[2]);
	float out = squareroot(&num);
	printf("%f\n", out);
	return 0;
};
