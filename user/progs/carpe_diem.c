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
  char *args[5] = { NULL };

  /*** BASIC TESTS ***/

  args[0] = "remove_pages_test1";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "remove_pages_test2";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "new_pages";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  char *sleep_args[] = {"sleep_test1", "20", NULL};
  if(!fork()) exec("sleep_test1", sleep_args);
  while(wait(NULL) >= 0);

  args[0] = "epileptic";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "cooperative_terminate";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "coolness_terminate";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "coy_terminate";
  if(!fork()) exec(args[0], args); 
  while(wait(NULL) >= 0);

  args[0] = "exec_basic";
  if(!fork()) exec(args[0], args); 
  while(wait(NULL) >= 0);

  args[0] = "fork_test1";
  if(!fork()) exec(args[0], args); 
  while(wait(NULL) >= 0);

  args[0] = "fork_wait";
  if(!fork()) exec(args[0], args); 
  while(wait(NULL) >= 0);

  args[0] = "getpid_test1";
  if(!fork()) exec(args[0], args); 
  while(wait(NULL) >= 0);

  args[0] = "loader_test1";
  if(!fork()) exec(args[0], args); 
  while(wait(NULL) >= 0);

  args[0] = "mem_eat_test";
  if(!fork()) exec(args[0], args); 
  while(wait(NULL) >= 0);

  args[0] = "print_basic";
  if(!fork()) exec(args[0], args); 
  while(wait(NULL) >= 0);

  args[0] = "stack_test1";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "wait_getpid";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "wild_test1";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  /*** SOLIDITY TESTS ***/

  args[0] = "yield_desc_mkrun";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);
   
  args[0] = "exec_nonexist";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "fork_exit_bomb";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "fork_wait_bomb";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "loader_test2";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "make_crash";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_basic_test";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_stands_for_swextensible";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_uninstall_test";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);


  args[0] = "swexn_regs";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_dispatch";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_cookie_monster";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "minclone_mem";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "mem_permissions";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  /*** STABILITY TESTS ***/

  args[0] = "cho";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);
  
  args[0] = "cho2";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "cho_variant";
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  /*** --- Classic Init Routine --- ***/

  int pid, exitstatus;
  char shell[] = "shell";
  args[0] = shell;
  args[1] = NULL;

  while(1) {
    pid = fork();
    if (!pid)
      exec(shell, args);
    
    while (pid != wait(&exitstatus));
  
    printf("Shell exited with status %d; starting it back up...", exitstatus);
  }
}
