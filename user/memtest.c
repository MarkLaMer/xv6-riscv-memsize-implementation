#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int before, after, shrunk;
  char *p;
  int pid;

  printf("=== Test 1: Basic sbrk growth ===\n");
  before = memsize();
  printf("memsize before sbrk: %d bytes\n", before);

  p = sbrk(4096);   // request one page
  if (p == (char*)-1) {
    printf("sbrk failed\n");
    exit(1);
  }
  
  // Touch the memory to ensure pages are allocated
  p[0] = 1;
  p[4095] = 1;

  after = memsize();
  printf("memsize after sbrk (after touch): %d bytes\n", after);
  printf("\n");

  printf("=== Test 2: Grow and shrink heap ===\n");
  printf("memsize before shrink: %d bytes\n", after);
  
  // Shrink the heap back
  p = sbrk(-4096);
  if (p == (char*)-1) {
    printf("sbrk(-4096) failed\n");
    exit(1);
  }

  shrunk = memsize();
  printf("memsize after sbrk(-4096): %d bytes\n", shrunk);
  printf("Original memsize: %d bytes\n", before);
  printf("Difference: %d bytes\n", shrunk - before);
  printf("\n");

  printf("=== Test 3: Separate address spaces with fork ===\n");
  before = memsize();
  printf("Parent memsize before fork: %d bytes\n", before);

  pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // Child process
    int child_before, child_after;
    char *child_p;

    child_before = memsize();
    printf("Child memsize before sbrk: %d bytes\n", child_before);

    child_p = sbrk(8192);  // allocate 2 pages in child
    if (child_p == (char*)-1) {
      printf("Child sbrk failed\n");
      exit(1);
    }
    
    // Touch the memory to ensure pages are allocated (2 pages)
    child_p[0] = 1;      // Touch first page
    child_p[4096] = 1;  // Touch second page
    child_p[8191] = 1;  // Touch last byte

    child_after = memsize();
    printf("Child memsize after sbrk(8192): %d bytes\n", child_after);
    printf("Child allocated: %d bytes\n", child_after - child_before);
    
    exit(0);
  } else {
    // Parent process
    int parent_after;
    int status;

    wait(&status);
    
    parent_after = memsize();
    printf("Parent memsize after child exits: %d bytes\n", parent_after);
    printf("Parent memsize unchanged: %s\n", 
           (parent_after == before) ? "yes" : "no");
    printf("This demonstrates separate address spaces!\n");
    printf("\n");
  }

  printf("=== Test 4: Virtual vs Physical Memory and Per-Process Isolation ===\n");
  printf("Experiment: Multiple children allocate memory independently\n");
  int parent_initial = memsize();
  printf("Parent initial memsize: %d bytes\n", parent_initial);
  
  // Fork and wait for each child sequentially to avoid output interleaving
  for (int i = 0; i < 3; i++) {
    pid = fork();
    if (pid < 0) {
      printf("fork failed\n");
      exit(1);
    }
    
    if (pid == 0) {
      // Child process
      int child_initial = memsize();
      char *ptr = sbrk((i + 1) * 4096);  // Each child allocates different amount
      
      if (ptr == (char*)-1) {
        printf("Child %d: sbrk failed\n", i);
        exit(1);
      }
      
      // Touch the memory to ensure pages are allocated
      // Touch at least one byte in each page
      for (int j = 0; j <= i; j++) {
        ptr[j * 4096] = 1;  // Touch first byte of each page
      }
      ptr[(i + 1) * 4096 - 1] = 1;  // Touch last byte of last page
      
      int child_after = memsize();
      printf("Child %d: initial=%d bytes, after sbrk(%d)=%d bytes, allocated=%d bytes\n",
             i, child_initial, (i+1)*4096, child_after, child_after - child_initial);
      exit(0);
    } else {
      // Parent waits for this child to complete before forking the next one
      wait(0);
    }
  }
  
  int parent_final = memsize();
  printf("Parent final memsize: %d bytes (unchanged from initial: %s)\n", 
         parent_final, (parent_final == parent_initial) ? "yes" : "no");
  printf("\n");
  printf("Explanation: The memsize values demonstrate virtual memory abstraction\n");
  printf("and per-process isolation - each child process allocates memory at the\n");
  printf("same virtual addresses but maps to different physical pages, and the\n");
  printf("parent's memsize remains unchanged, showing complete memory isolation.\n");

  exit(0);
}
