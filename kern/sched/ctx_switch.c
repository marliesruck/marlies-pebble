/** @files ctx_switch.c
 *
 *  @brief C routine for context switching.
 *
 *  Store out the running thread's context and add to the runnable queue.
 *  Dequeue the head of the runnable queue.  
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
  curr = thrlist_dequeue(&naive_thrlist);

  asm_ctx_switch(&prev->sp, &prev->pc, curr->sp, 
                  curr->pc, curr->task_info->cr3, 
                  (&curr->kstack[KSTACK_SIZE -1]));
  return;
}
