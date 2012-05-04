
#include <crest.h>
#include <stdio.h>

int main(void) {
	int x, y;
	CREST_int(x);
	CREST_int(y);

	if (x < 10) {
		printf("x < 10\n");
	} else if (x > 15 ) {
		printf("x > 15 \n");
	} else {
		printf("10 <= x <= 15\n");
	}

	if (y > 20) {
		printf("y > 20\n");
	} else {
		printf("y <= 20)\n");
	}
	return 0;
}
