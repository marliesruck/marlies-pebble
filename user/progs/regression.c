/** @file user/progs/init_2.0.c
 *
 *  Runs a test suite BEFORE launching init.
 *
 *  @status done
 */

#include <syscall.h>
#include <stdio.h>
#include <simics.h>

int main()
{
  /*** --- Regression tests --- ***/

  if(!fork()){
    exec("cooperative_terminate", NULL);
  }

  while(wait(NULL));

  if(!fork()){
    exec("coolness_terminate", NULL);
  }

  while(wait(NULL));

  if(!fork()){
    exec("coy_terminate", NULL);
  }

  while(wait(NULL));

  /*** --- Classic Init Routine --- ***/

  int pid, exitstatus;
  char shell[] = "shell";
  char * args[] = {shell, 0};

  while(1) {
    pid = fork();
    if (!pid)
      exec(shell, args);
    
    while (pid != wait(&exitstatus));
  
    printf("Shell exited with status %d; starting it back up...", exitstatus);
  }
}
