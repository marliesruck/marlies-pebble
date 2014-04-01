/** @file misc_syscalls.c
 *
 *  @brief Implements our miscellaneous system calls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

/* Pebbles includes*/
#include <dispatch.h>
#include <mutex.h>
#include <sc_utils.h>
#include <sched.h>
#include <ureg.h>
#include <util.h>

/* Libc specific includes */
#include <stdlib.h>
#include <string.h>

/* x86 specific includes */
#include <x86/asm.h>
#include <x86/cr.h>

#define NUM_REGS 20
/*************************************************************************
 *  Internal helper functions
 *************************************************************************/

/** @brief Adopt register values requested by user.
 *
 *  @param state Execution state to be adopted.
 *
 *  @return Void.
 **/
void craft_state(ureg_t state);

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

/* @bug Argument validation of pointers */

int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
  /* Validate register values */
  if(newureg){
    if(validate_regs(newureg) < 0){
      return -1;
    }
  }

  /* Validate and install new handler */
  if((eip) && (esp3)){
    if((sc_validate_argp(eip, 1)) || (sc_validate_argp(esp3, 1))){
      return -1;
    }
    curr->swexn.esp3 = esp3;
    curr->swexn.eip = eip;
    curr->swexn.arg = arg;
  }
  /* Or deregister old handler */
  else{
    deregister(&curr->swexn);
  }

  /* Install the registers and return to userland */
  if(newureg){
    craft_state(*newureg);
  }

  /* Or simply return */
  return 0;
}


/* "Special" */
void sys_misbehave(int mode)
{
  return;
}

