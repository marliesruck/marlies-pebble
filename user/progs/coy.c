/* @file coy.c
 *
 * Basic stress test for set status, wait, fork, vanish, gettid.
 *
 * Parent waits on child and reports status.  Child sets its status to its tid
 * and vanishes.
 */
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>

int main(){
  lprintf("dereferncing NULL...");
  int *test = (int *)(NULL);
  *test = 5;
  lprintf("done");
  int tid, status;
  while(1){
    if((tid = fork()) != 0){
      lprintf("1");
      wait(&status);
      lprintf("2");
    }
    else{
        vanish();
    }
  }
}
