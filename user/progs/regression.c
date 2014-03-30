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

  if(!fork()) exec("minclone_mem", NULL);
  while(wait(NULL));

  if(!fork()) exec("new_pages", NULL);
  while(wait(NULL));

  if(!fork()) exec("remove_pages_test1", NULL);
  while(wait(NULL));

  /* TODO: Uncomment when we kill faulting processes */
//  if(!fork()) exec("remove_pages_test2", NULL);
//  while(wait(NULL));

  /*
  char *sleep_args[] = {"sleep_test1", "20", NULL};
  if(!fork()) exec("sleep_test1", sleep_args);
  while(wait(NULL));

  if(!fork()) exec("epileptic", NULL);
  while(wait(NULL));
  */

  if(!fork()) exec("cooperative_terminate", NULL);
  while(wait(NULL));

  if(!fork()) exec("coolness_terminate", NULL);
  while(wait(NULL));

  if(!fork()) exec("coy_terminate", NULL); 
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
