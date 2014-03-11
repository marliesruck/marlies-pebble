/** @files sched.c
 *
 *  @brief Implements our scheduler.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <sched.h>

#include <ctx_switch.h>
#include <process.h>
#include <thread.h>

#include <assert.h>
#include <simics.h>


/** @var runnable
 *  @brief The queue of runnable threads.
 **/
queue_s runnable = QUEUE_INITIALIZER(runnable);

/** @var blocked
 *  @brief A list of currently un-runnable threads
 **/
cll_list blocked = CLL_LIST_INITIALIZER(blocked);

int sched_block(int tid)
{
  cll_node *n;
  thread_t *thr;

  /* Find the blockee in the runnable queue
   * We don't need to check curr; that's us
   */
  cll_foreach(&runnable, n) {
    thr = queue_entry(thread_t *, n);
    if (thr->tid == tid) break;
  }
  if (n == &runnable) return -1;

  /* Add the blockee node to the blocked list
   * We must use cll_extract(...) here as the queue API only allows
   * dequeueing.
   */
  n = cll_extract(&runnable, n);
  cll_insert(&blocked, n);

  return 0;
}

int sched_unblock(int tid)
{
  cll_node *n;
  thread_t *thr;

  /* Find the blockee in the blocked list */
  cll_foreach(&blocked, n) {
    thr = cll_entry(thread_t *, n);
    if (thr->tid == tid) break;
  }
  if (n == &blocked) return -1;

  /* Add the blocked node to the runnable queue */
  n = cll_extract(&blocked, n);
  queue_enqueue(runnable.prev, n);

  return 0;
}

void schedule(void)
{
  thread_t *prev, *next;
  queue_node_s *q;

  if (queue_empty(&runnable)) return;

  /* Dequeue the lucky thread */
  q = queue_dequeue(&runnable);
  next = queue_entry(thread_t *, q);

  /* Enqueue the currently running thread */
  prev = curr;
  queue_init_node(q,prev);
  queue_enqueue(&runnable, q);

  /* Keep track of who's running */
  assert(next != curr);
  curr = next;

  asm_ctx_switch(&prev->sp, &prev->pc, next->sp, 
                 next->pc, next->task_info->cr3, 
                 &next->kstack[KSTACK_SIZE]);

  return;
}

