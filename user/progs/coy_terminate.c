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
#include <report.h>
#include "../../410user/inc/410_tests.h"

#define LIM 32

static int count = 0;
DEF_TEST_NAME("coy:");

int main(){
  int tid, status;
  report_start(START_CMPLT);
  
  while(count < LIM){
    if((tid = fork()) != 0){
      lprintf("parent waiting for child");
      wait(&status);
      lprintf("status = %d", status);
      count++;
    }
    else{
        lprintf("in child setting status");
        set_status(gettid());
        lprintf("In child vanishing");
        vanish();
    }
  }
  report_end(END_SUCCESS);
  vanish();
}
