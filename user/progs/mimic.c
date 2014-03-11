/** @file introvert.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */

#define LEN 100

int main()
{
  char line[LEN+1];
  int read;

  while(1){
    read = readline(LEN, line);
    line[LEN] = '\0';
    lprintf("read %d characters: %s", read, line);
  }
}
