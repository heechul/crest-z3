#include <crest.h>
#include <stdio.h>

int
main (int argc, char *argv[])
{
  int x;
  int A[] = { 0, 1, 2, 3, 4, 5 };
  CREST_int (x);
  if (A[x] == 2) {
      printf ("GOAL\n");
  }
  return 0;
}
