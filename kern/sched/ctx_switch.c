#include <ctx_switch.h>
#include <sched.h>

void half_ctx_switch_wrapper(void)
{
  /* Function is not ctx switch safe beyond this point */
  disable_interrupts();

  /* Remove yourself from the runnable queue */
  remove_from_runnable(curr);

  /* There should still be someone to switch to */
  assert( !queue_empty(&runnable) );

  /* Dequeue the thread we are switching to */
  queue_node_s *q = queue_dequeue(&runnable);
  thread_t *next = queue_entry(thread_t *, q);

  /* Move the thread to the back of the queue */
  assert(next->state == THR_RUNNING);
  queue_enqueue(&runnable, q);

  curr = next; 

  half_ctx_switch(next->sp, next->pc, next->task_info->cr3,
                  &next->kstack[KSTACK_SIZE]);

}
