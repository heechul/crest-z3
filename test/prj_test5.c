#include <crest.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int x;

	CREST_int(x);

	if (x > 0) {
		if (x/2 == 2) {
			printf("x/2=2\n");
		}
	}
	return 0;
}
