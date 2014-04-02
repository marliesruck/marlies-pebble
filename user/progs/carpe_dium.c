/** @file user/progs/carpe_dium.c
 *
 *  A battery of user tests we currently pass, plus the ones we don't pass
 *  commented out.
 *
 *  @status done
 */

#include <syscall.h>
#include <stdio.h>
#include <simics.h>

int main()
{
  /*** BASIC TESTS ***/

  if(!fork()) exec("remove_pages_test1", NULL);
  while(wait(NULL) >= 0);

  char *sleep_args[] = {"sleep_test1", "20", NULL};
  if(!fork()) exec("sleep_test1", sleep_args);
  while(wait(NULL) >= 0);

  if(!fork()) exec("epileptic", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("cooperative_terminate", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("coolness_terminate", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("coy_terminate", NULL); 
  while(wait(NULL) >= 0);

  if(!fork()) exec("exec_basic", NULL); 
  while(wait(NULL) >= 0);

  if(!fork()) exec("fork_test1", NULL); 
  while(wait(NULL) >= 0);

  if(!fork()) exec("fork_wait", NULL); 
  while(wait(NULL) >= 0);

  if(!fork()) exec("getpid_test1", NULL); 
  while(wait(NULL) >= 0);

  if(!fork()) exec("loader_test1", NULL); 
  while(wait(NULL) >= 0);

  if(!fork()) exec("mem_eat_test", NULL); 
  while(wait(NULL) >= 0);

  if(!fork()) exec("print_basic", NULL); 
  while(wait(NULL) >= 0);

  if(!fork()) exec("readline_basic", NULL); 
  while(wait(NULL) >= 0);

  if(!fork()) exec("stack_test1", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("wait_getpid", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("wild_test1", NULL);
  while(wait(NULL) >= 0);

  /*** SOLIDITY TESTS ***/

  /* Uncomment this after implementing yield:
  if(!fork()) exec("yield_desc_mkrun", NULL);
  while(wait(NULL) >= 0);
  */
   
  /* This tests runs forever...but we fail an assertion in vm copy
  if(!fork()) exec("fork_bomb", NULL);
  while(wait(NULL) >= 0);
  */

  if(!fork()) exec("exec_nonexist", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("fork_exit_bomb", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("fork_wait_bomb", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("loader_test2", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("make_crash", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("swexn_basic_test", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("swexn_stands_for_swextensible", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("swexn_uninstall_test", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("remove_pages_test2", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("swexn_regs", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("swexn_dispatch", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("swexn_cookie_monster", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("minclone_mem", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("new_pages", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("mem_permissions", NULL);
  while(wait(NULL) >= 0);

  /*** STABILITY TESTS ***/

  if(!fork()) exec("cho", NULL);
  while(wait(NULL) >= 0);
  
  /*
  if(!fork()) exec("cho2", NULL);
  while(wait(NULL) >= 0);

  if(!fork()) exec("cho_variant", NULL);
  while(wait(NULL) >= 0);

  */


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
