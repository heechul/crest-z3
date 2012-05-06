
#include <crest.h>
#include <stdio.h>

int main(void) {
	int x, y;
	CREST_int(x);
	CREST_int(y);

	if (y > 20) {
		if (x > 15 ) {
			printf("GOAL\n");
		} 
	} 
	return 0;
}
