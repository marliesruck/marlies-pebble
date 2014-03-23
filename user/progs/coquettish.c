/* @file coquettish.c
 *
 */
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>

int main(){
  int tid = fork();
  if(tid){
    int status;
    lprintf("parent waiting for child");
    wait(&status);
    lprintf("status = %d", status);
    while(1);
  }

  else{
      lprintf("in child setting status");
      set_status(23);
      lprintf("In child vanishing");
      vanish();
  }
}
