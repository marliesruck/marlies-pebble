/** @file introspective.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */

int main(int argc, char **argv)
{
  int i;

  lprintf("Who am I?  I'm introspective!");
  lprintf("My tid is %d, and my args are...", gettid());

  for (i = 0; i < argc; ++i)
    lprintf("  argv[%d] = %s", i, argv[i]);

  while (1) continue;
}
