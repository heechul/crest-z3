#include <crest.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int x, y;

	CREST_int(x);
	CREST_int(y);

	if (x * y == 49) {
		printf("GOAL x=%d, y=%d\n", x, y);
	}
	return 0;
}
