/** @file introvert.c
 **/

#include <syscall.h>  /* for getpid */
#include <simics.h>    /* for lprintf */

int test[4096];

int main()
{
  lprintf("&test[4095]:%p test[4095]: %d",&test[4095], test[4095]);
  test[4095] = 123;
  lprintf("test[4095] = %d",test[4095]);
  //MAGIC_BREAK;
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
