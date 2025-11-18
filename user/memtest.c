#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int before, after;
  char *p;

  before = memsize();
  printf("memsize before sbrk: %d bytes\n", before);

  p = sbrk(4096);   // request one page
  if (p == (char*)-1) {
    printf("sbrk failed\n");
    exit(1);
  }

  after = memsize();
  printf("memsize after sbrk: %d bytes\n", after);

  exit(0);
}
