/** @file libautostack.c
 *
 *  @brief Implement's automatic stack extension.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 **/

#include <stack.h>

#include <assert.h>
#include <malloc.h>
#include <syscall.h>

/** @brief Handle stack growth for single-threaded programs.
 *
 *  @param stack_low The current lowest stack address.
 *  @param ureg User-space register values at the time of the exception.
 *
 *  @return Void.
 **/
void stackgrowth_handler(void *stack_low, ureg_t *ureg)
{
  unsigned sp_low = (unsigned int) stack_low;
  int result;

  /* We're only interested in read or write page faults */
  if (ureg->cause != SWEXN_CAUSE_PAGEFAULT)
    swexn(NULL, NULL, NULL, ureg);

  /* If ESP hasn't passed the bottom of the stack, not our problem */
  else if (ureg->esp >= sp_low) swexn(NULL, NULL, NULL, ureg);

  /* If the faulting address isn't on the stack, also not our problem */
  else if (ureg->cr2 >= sp_low && ureg->cr2 < ureg->esp)
    swexn(NULL, NULL, NULL, ureg);

  /* Otherwise, try to allocate more stack space */
  result = new_pages((void *)(sp_low - PAGE_SIZE), PAGE_SIZE);
  if (result != 0) {
    swexn(NULL, NULL, NULL, ureg);
  }

  /* Register a handler for next time */
  swexn(exn_stack + EXN_STACK_SIZE, stackgrowth_handler,
        (void *)sp_low - PAGE_SIZE, ureg);

  /* swexn(...) won't return */
}

#include <simics.h>
void install_autostack(void *stack_high, void *stack_low)
{
  /* Save the high/low stack addresses */
	sp_high = stack_high;
	sp_low = stack_low;

  /* Install the exception handler */
  swexn(exn_stack+EXN_STACK_SIZE, stackgrowth_handler, stack_low, NULL);

  return;
}

