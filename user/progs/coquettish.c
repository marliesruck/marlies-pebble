/* @file vanish_test.c
 *
 */
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>

int main(){
  /*
  char buf[16];
  char * prog = "merchant";
  char * args[5] = { "merchant", "13", "foo bar", NULL, NULL };

  args[3] = buf;

  int tid = fork();

  if(tid == 0){
    exec(prog,args);
  }

  */
  /* Fork again */
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
