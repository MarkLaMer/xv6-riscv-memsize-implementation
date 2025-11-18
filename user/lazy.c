#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int before, mid, after;
  int *a;

  before = memsize();
  printf("memsize before sbrk: %d bytes\n", before);

  //a = sbrk(4096);   // grow by one page
  a = (int*) sbrk(4096);   // grow by one page
  printf("sbrk returned %p\n", a);

  mid = memsize();
  printf("memsize after sbrk (before touch): %d bytes\n", mid);

  printf("writing into new page...\n");
  a[0] = 123;       // should trigger page fault and allocation

  after = memsize();
  printf("memsize after touch: %d bytes\n", after);
  printf("a[0] = %d\n", a[0]);

  exit(0);
}
