/** @file misc_syscalls.c
 *
 *  @brief Implements our miscellaneous system calls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

/* Pebbles includes*/
#include <ureg.h>


/*************************************************************************
 *  Miscellaneous system calls
 *************************************************************************/

void sys_halt()
{
  return;
}

int sys_readfile(char *filename, char *buf, int count, int offset)
{
  return -1;
}

typedef void (*swexn_handler_t)(void *arg, ureg_t *ureg);
int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
  return -1;
}


/* "Special" */
void sys_misbehave(int mode)
{
  return;
}

