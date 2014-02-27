/** @file schizo.c
 **/

#include <syscall.h>
#include <simics.h>
#include <stddef.h>


int main()
{
  lprintf("exec'ing...");
  exec("introspective", NULL);
	
  while (1) continue;
  return 0;
}

