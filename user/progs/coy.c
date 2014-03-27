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
  int tid;
  while(1){
    if(fork()){
      tid = wait(NULL);
      lprintf("parent reaped %d", tid);
    }
    else{
        vanish();
    }
  }
}
