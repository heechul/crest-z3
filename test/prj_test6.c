#include <crest.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	int x;

	CREST_int(x);

	if (x > 10 && x < 20) {
		if (x%10 == 3) {
			printf("x%10=3, x=%d\n", x);
		}
	}
	return 0;
}
