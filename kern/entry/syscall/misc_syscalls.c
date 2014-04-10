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
#include <assert.h>
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

/** @brief Shut down the system.
 *
 *  @return Does not return.
 **/
void sys_halt()
{
  sim_halt();

  disable_interrupts();

  lprintf("Execution ceased");

  while(1);

  assert(0);
}

/** @brief Read bytes from a file.
 *
 *  Read count bytes from position offset in the file filename.  Bytes read
 *  from the file will be copied into buf.  If fewer than count bytes
 *  remain in the file starting at offset, only that many bytes are read.
 *
 *  The number of bytes read into buf is returned.  If buf is in invaid
 *  memory, or if file does not exist, an error code is returned.
 *
 *  @param filename The name of the file to read from.
 *  @param buf The buffer to copy file data into.
 *  @param count The number of bytes to copy.
 *  @param offset The offset in the file to start reading from.
 **/
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

/** @brief Register/deregister software exception handlers, and set
 *  register values.
 *
 *  If esp3 and eip are bot non-zero, attempt to register a new software
 *  exception handler; otherwise deregister the current handler, if one
 *  exists.
 *
 *  If newureg is non-zero, the specified register values are adopted upon
 *  the return of sys_swexn(...) to user-space.
 *
 *  @param esp3 The software exception stack.
 *  @param eip The address of the software exception handler.
 *  @param arg The arguments to the software exception handler.
 *  @param newureg New register values the kerenl should adopt.
 *
 *  @return 0 on success, or a negative integer error code on failure.
 **/
int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
  ureg_t ureg; 

  /* Copy and validate register values */
  if(newureg) {
    if(copy_from_user_static(&ureg, newureg, sizeof(ureg_t)))
      return -1;
    if(validate_regs(&ureg) < 0)
      return -1;
  }

  /* Validate and install new handler */
  if((eip) && (esp3))
  {
    if((validate_pc(eip)) || (validate_sp(esp3))) {
      return -1;
    }
    curr_thr->swexn.esp3 = esp3;
    curr_thr->swexn.eip = eip;
    curr_thr->swexn.arg = arg;
  }

  /* Or deregister old handler */
  else
  {
    swexn_deregister(&curr_thr->swexn);
  }

  /* Install the registers and return to userland */
  if(newureg) craft_state(ureg);

  return 0;
}


/** @brief Changes the behavior of the kernel.
 *
 *  NOTE: unimplemented.
 *
 *  @param mode The desired behavior mode.
 *
 *  @return Void.
 **/
void sys_misbehave(int mode)
{
  return;
}

