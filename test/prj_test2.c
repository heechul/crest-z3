
#include <crest.h>
#include <stdio.h>

int main(void) {
	int x;
	CREST_int(x);

	if (x < 10) {
		printf("GOAL\n");
	} else if (x > 15 ) {
		printf("GOAL1\n");
	} else {
		printf("GOAL2\n");
	}
	return 0;
}
