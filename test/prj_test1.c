
#include <crest.h>
#include <stdio.h>

int main(void) {
	int x;
	CREST_int(x);

	if (x < 10) {
		printf("GOAL\n");
	} else {
		printf("GOAL1\n");
	}
	return 0;
}
