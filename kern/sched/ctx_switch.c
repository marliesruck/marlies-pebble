/** @files ctx_switch.c
 *
 *  @brief C routine for context switching.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug As of now we have only implemented a naive thread list
 **/
#include <ctx_switch.h>
#include <process.h>
#include <thread.h>

#include <simics.h>

void ctx_switch(void)
{
  thread_t *prev = curr;
  thrlist_enqueue(prev, &naive_thrlist);
  thread_t *next = thrlist_dequeue(&naive_thrlist);
  curr = next;

  asm_ctx_switch(&prev->sp, &prev->pc, next->sp, 
                 next->pc, next->task_info->cr3, 
                 &next->kstack[KSTACK_SIZE]);

  return;
}

