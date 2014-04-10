/** @files dispatch.c
 *
 *  @brief Defines C wrapper for the assembly routine dispatch().
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#include <simics.h>

#include <dispatch.h>

/* Pebbles specific includes */
#include <thread.h>
#include <sched.h>

/* Libc specific */
#include <assert.h>

/* x86 specific include */
#include <asm.h>

void dispatch(thread_t *next)
{
  thread_t *prev;
  unsigned int cr3 = 0;

  /* Update the current task and cr3 */
  if(curr_tsk != next->task_info){
    cr3 = next->task_info->cr3;
    curr_tsk = next->task_info;
  }

  /* Update the current thread */
  prev = curr_thr;
  curr_thr = next;

  asm_dispatch(&prev->sp, &prev->pc, next->sp, next->pc, cr3, 
           &next->kstack[KSTACK_SIZE]);
  return;
}
