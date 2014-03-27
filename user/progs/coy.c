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
    lprintf("forking");
    if((tid = fork()) != 0){
      lprintf("waiting");
      wait(&status);
      lprintf("reaped child with status = %d", status);
    }
    else{
        lprintf("setting status");
        set_status(gettid());
        lprintf("vanishing");
        vanish();
    }
  }
}
