/** @file pagefault.c
 *  @brief A simple test that recurses until it pagefaults.
 *  @author Enrique Naudon (esn)
 **/

#include <simics.h>

#include <stddef.h>
#include <string.h>
#include <syscall.h>

#define SIZE PAGE_SIZE/4

int recurse(int n)
{
  char arr[SIZE];
  memset(arr, 0, SIZE);
  lprintf("recurse: n = %d", n);
  return recurse(++n);
}

int main()
{
  return recurse(0);
}

