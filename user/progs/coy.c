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
  int tid, status;
  while(1){
    if((tid = fork()) != 0){
      lprintf("parent waiting for child");
      wait(&status);
      lprintf("status = %d", status);
    }
    else{
        lprintf("in child setting status");
        set_status(gettid());
        lprintf("In child vanishing");
        vanish();
    }
  }
}
