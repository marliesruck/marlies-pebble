/** @file introvert.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */

int main()
{
  lprintf("Hi...");

  while (1) continue;
}
