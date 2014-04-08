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
  char *args[5];

  /*** --- Regression tests --- ***/

  args[0] = "cho_variant";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_basic_test";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_stands_for_swextensible";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_uninstall_test";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "remove_pages_test2";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_regs";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_dispatch";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "swexn_cookie_monster";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "minclone_mem";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "new_pages";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "remove_pages_test1";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "sleep_test1";
  args[1] = "20";
  args[2] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "epileptic";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "cooperative_terminate";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "coolness_terminate";
  args[1] = NULL;
  if(!fork()) exec(args[0], args);
  while(wait(NULL) >= 0);

  args[0] = "coy_terminate";
  args[1] = NULL;
  if(!fork()) exec(args[0], args); 
  while(wait(NULL) >= 0);

  return 0;
}
