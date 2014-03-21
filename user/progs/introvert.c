/** @file introvert.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */

int test[4096];

int main()
{
  int tid = fork();
  if(tid)
    lprintf("parent");
  else
    lprintf("child");

  if((tid = fork()) == 0)
    lprintf("parent");
  else
    lprintf("child");

  while(1);
  /*
  lprintf("forking...");
  int tid = fork();
  lprintf("returned from fork!");
  if(tid){
    lprintf("in parent with tid %d!",tid);
  }
  else
    lprintf("in child!");


  while(1){
    while(getchar() != 'r');
    lprintf("fork returned tid: %d",tid);
  }
  */
}
