
#include <crest.h>
#include <stdio.h>

int main(void) {
	int x;
	CREST_int(x);

	if (x < 10) {
		printf("x < 10\n");
	} else {
		printf("x > 10\n");
	}
	return 0;
}
