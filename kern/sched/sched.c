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
#include <malloc.h>
#include <simics.h>


/** @var runnable
 *  @brief The queue of runnable threads.
 **/
queue_s runnable = QUEUE_INITIALIZER(runnable);

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

  /* Remove the blockee node from the runnable queue
   * We must use cll_extract(...) here as the queue API only allows
   * dequeueing
   */
  assert(cll_extract(&runnable, n) == n);
  free(n);

  thr->state = THR_BLOCKED;

  return 0;
}

int sched_unblock(int tid)
{
  queue_node_s *n;
  thread_t *thr;

  /* Find the blocked node */
  thr = thrlist_find(tid);
  if (!thr) return -1;

  /* Add the blocked node to the runnable queue */
  n = malloc(sizeof(cll_node));
  if (!n) return -1;
  queue_init_node(n, thr);
  queue_enqueue(&runnable, n);

  thr->state = THR_RUNNING;

  return 0;
}

/** @brief Maybe run someone new for a while.
 *
 *  This is our main scheduling function.  It selects a thread from the
 *  runnable queue and switches to that thread.
 *
 *  We avoid malloc'ing in this function by reusing the dequeue'd (i.e.
 *  about-to-be-run) thread's queue node to enqueue the thread we remove
 *  from the CPU.
 *
 *  @return Void.
 **/
void schedule(void)
{
  thread_t *prev, *next;
  queue_node_s *q;

  if (queue_empty(&runnable)) return;

  /* Dequeue the lucky thread */
  q = queue_dequeue(&runnable);
  next = queue_entry(thread_t *, q);
  assert(next->state == THR_RUNNING);

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

