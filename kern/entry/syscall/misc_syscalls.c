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
#include <loader.h>
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
  char *filename_k, *buf_k;
  int copied;

  /* Allocate kernel memory for user arguments */
  buf_k = malloc(count * sizeof(char));
  if (!buf_k) return -1;

  /* Copy user arguments
   * TODO: validate file?
   */
  if ((copy_from_user(&filename_k, filename, strlen(filename) + 1))
      /*|| (validate_file(&se, filename_k) < 0) */)
  {
    free(buf_k);
    free(filename_k);
    return -1;
  }

  copied = getbytes(filename_k, offset, count, buf_k);

  /* Give it to the user */
  if (copy_to_user(buf, buf_k, copied)) {
    free(buf_k);
    free(filename_k);
    return -1;
  }

  free(buf_k);
  free(filename_k);
  return copied;
}

/* @bug Argument validation of pointers 
 *
 * Potential loop: Install handler, crash, handler is called and reinstalls
 * itself and replaces the registers with the exact same values
 *
 * */


int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
  ureg_t *ureg;

  if(copy_from_user((char **)&ureg, (char *)newureg, sizeof(ureg_t)))
    return -1;

  /* Validate register values */
  if(ureg){
    if(validate_regs(ureg) < 0){
      free(ureg);
      return -1;
    }
  }

  /* Validate and install new handler */
  if((eip) && (esp3)){
    if((validate_pc(eip)) || (validate_sp(esp3))){
      free(ureg);
      return -1;
    }
    curr_thr->swexn.esp3 = esp3;
    curr_thr->swexn.eip = eip;
    curr_thr->swexn.arg = arg;
  }
  /* Or deregister old handler */
  else{
    deregister(&curr_thr->swexn);
  }

  /* Install the registers and return to userland */
  if(ureg){
    /* TODO: FIX MEMORY LEAK, ureg should be on stack */
    craft_state(*ureg);
  }

  /* Or simply return */
  free(ureg);
  return 0;
}


/* "Special" */
void sys_misbehave(int mode)
{
  return;
}

