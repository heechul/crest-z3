
#include <crest.h>
#include <stdio.h>

int main() {
    int x;
    int A[] = { 0, 1, 2, 3, 4, 5};
    CREST_int(x);

    if ( x > 0 ) {

	if ( x - ( x / 3 * 3) == 0) {
	    printf("c1: x % 3 == 0 \n");
	} else {
	    printf("c2: x % 3 != 0 \n"); 
	}
#if 1
	if ( A[x] == 2 ) {
	    printf("c3: x>0 && A[x] == 2\n");
	} else {
	    printf("c4: x>0 && A[x] != 2\n");
	}
#endif
    } else {
	printf("c0: x < 0 \n");
    }
}
