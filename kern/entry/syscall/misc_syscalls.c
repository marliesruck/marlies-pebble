/** @file misc_syscalls.c
 *
 *  @brief Implements our miscellaneous system calls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

#include "swexn.h"

/* Pebbles includes*/
#include <mutex.h>
#include <sc_utils.h>
#include <sched.h>
#include <ureg.h>
#include <util.h>

/* Libc specific includes */
#include <stdlib.h>

/* x86 specific includes */
#include <x86/asm.h>

#define NUM_REGS 20

void *swexn_sp = NULL;
swexn_handler_t swexn_fun = NULL;
void *swexn_arg = NULL;

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

/** @brief Set up exception stack and deregister handler.
 *
 *  @param state Execution state to push on exception stack.
 *
 *  @return Void.
 **/
void init_exn_stack(ureg_t *state)
{
  /* Store out handler to call */
  swexn_handler_t eip = swexn_fun;
  void *esp3 = swexn_sp;
  void *arg = swexn_arg;

  /* Deregister old handler */
  swexn_sp = NULL;
  swexn_fun = NULL;
  swexn_arg = NULL;
  
  /* Craft contents of exception stack */
  PUSH(esp3, state);    /* Executation state */
  PUSH(esp3, arg);      /* Opaque void * arg */
  PUSH(esp3, 0);        /* Dummy return address */

  /* Run handler */
  run_handler(eip, esp3);

  return;
}

/* @bug Argument validation */

int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
  /* Validate register values */
  if(newureg){
    if(sc_validate_argp(newureg, NUM_REGS)){
      return -1;
    }
  }

  /* Validate and install new handler */
  if((eip) && (esp3)){
    if((sc_validate_argp(eip, 1)) || (sc_validate_argp(esp3, 1))){
      return -1;
    }
    swexn_sp = esp3;
    swexn_fun = eip;
    swexn_arg = arg;
  }
  /* Or deregister old handler */
  else{
    swexn_sp = NULL;
    swexn_fun = NULL;
    swexn_arg = NULL;
  }

  /* Install the registers and return to userland */
  if(newureg){
    disable_interrupts();
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

