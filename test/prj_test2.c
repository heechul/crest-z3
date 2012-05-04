
#include <crest.h>
#include <stdio.h>

int main(void) {
	int x;
	CREST_int(x);

	if (x < 10) {
		printf("x < 10\n");
	} else if (x > 15 ) {
		printf("x > 15 \n");
	} else {
		printf("10 <= x <= 15\n");
	}
	return 0;
}
