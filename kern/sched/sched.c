/** @files sched.c
 *
 *  @brief Implements our scheduler.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <simics.h>
#include <sched.h>

/* Pebble specific includes */
#include <dispatch.h>

/* Libc specific includes */
#include <assert.h>
#include <malloc.h>

/* x86 includes */
#include <asm.h>

/** @var runnable
 *  @brief The queue of runnable threads.
 **/
queue_s runnable = QUEUE_INITIALIZER(runnable);


/*************************************************************************
 *  Runqueue manipulation
 *************************************************************************/

/** @brief Make a thread eligible for CPU time.
 *
 *  This operation is not atomic.
 *
 *  @param n The node to add to the runnable queue.
 *  @param thr The thread to make runnable.
 *
 *  @return Void. 
 **/
void rq_add(thread_t *thr)
{
  assert(thr->state != THR_RUNNABLE);
  queue_enqueue(&runnable, &thr->rq_entry);
  thr->state = THR_RUNNABLE;

  return;
}

/** @brief Make a thread ineligible for CPU time.
 *
 *  This function uses the linked list API instead of the queue API,
 *  because the queue API does not allow extraction of arbitrary nodes.
 * 
 *  @param thr The thread to make unrunnable.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
void rq_del(thread_t *thr)
{
  assert(thr->state == THR_RUNNABLE);
  assert(cll_extract(&runnable, &thr->rq_entry));
  thr->state = THR_BLOCKED;
  
  return;
}

/** @brief Move the head of the run queue to the back.
 *
 *  @return A pointer to the TCB of the rotated thread, or NULL if the
 *  queue is empty.
 **/
thread_t *rq_rotate(void)
{
  queue_node_s *q;
  thread_t *thr;

  /* Return NULL for an empty run queue */
  if (queue_empty(&runnable)) return NULL;

  /* Move the head to the back of the queue */
  q = queue_dequeue(&runnable);
  thr = queue_entry(thread_t *, q);
  assert(thr->state == THR_RUNNABLE);
  queue_enqueue(&runnable, q);

  return thr;
}

/** @brief Search the runqueue for a particular TID.
 *
 *  This function uses the linked list API instead of the queue API,
 *  because the queue API does not allow searching.
 * 
 *  @param tid The TID of the target thread.
 *
 *  @return A pointer to the thread's TCB on success; NULL on error.
 **/
thread_t *rq_find(int tid)
{
  thread_t *thr;
  cll_node *n;

  cll_foreach(&runnable, n) {
    thr = queue_entry(thread_t *, n);
    assert(thr->state == THR_RUNNABLE);
    if (thr->tid == tid) return thr;
  }

  return NULL;
}


/*************************************************************************
 *  The scheduler
 *************************************************************************/

/** @brief Make a thread eligible for CPU time.
 *
 *  This operation is atomic.
 *
 *  @param thr The thread to make runnable.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int sched_add_to_rq(thread_t *thr)
{
  /* Don't malloc a node, instead use the embedded list traversal structure */

  /* Lock, enqueue, unlock */
  disable_interrupts();
  rq_add(thr);
  enable_interrupts();

  return 0;
}

/** @brief Make a thread eligible for CPU time.
 *
 *  This operation is atomic.
 *
 *  @param thr The thread to make runnable.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
void sched_unblock(thread_t *thr)
{
  disable_interrupts();

  rq_add(thr);
  schedule_unprotected();

  enable_interrupts();
  return;
}

/** @brief Make a thread ineligible for CPU time.
 *
 *  @param thr The thread to make unrunnable.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
void sched_block(thread_t *thr)
{
  disable_interrupts();

  rq_del(thr);
  schedule_unprotected();

  enable_interrupts();
  return;
}

/** @brief Atomically execute a caller-specified function and block.
 *
 *  @param thr The thread to make unrunnable.
 *  @param fn The function to execute before blocking.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
void sched_do_and_block(thread_t *thr, sched_do_fn func, void *args)
{
  disable_interrupts();

  func(args);
  rq_del(thr);
  schedule_unprotected();

  enable_interrupts();
  return;
}

/** @brief Defer execution of the invoking thread in favor of another thread
 * specified by tid.
 *
 *  @param tid Thread to defer to.
 *
 *  @return -1 If thread specificed by tid is ineligible for CPU time, else 0.
 **/
int sched_find(int tid)
{
  thread_t *thr;

  /* Find the target thread */
  disable_interrupts();
  thr = rq_find(tid);

  /* Return error if the thread is not runnable */
  if (!thr) {
    enable_interrupts();
    return -1;
  }

  schedule_unprotected();
  enable_interrupts();
  return 0;
}

/** @brief Try to run someone new.
 *
 *  @return Void.
 **/
void schedule_unprotected(void)
{
  thread_t *next;

  /* The runnable queue should never be empty */
  assert( next = rq_rotate() );

  /* Only switch if the next thread is different */
  if (next != curr_thr) {
    dispatch_wrapper(next);
  }

  return;
}

/** @brief Maybe run someone new for a while.
 *
 *  This is our main scheduling function.  It selects a thread from the
 *  runnable queue and switches to that thread.
 *
 *  @return Void.
 **/
void schedule(void)
{
  disable_interrupts();
  schedule_unprotected();
  enable_interrupts();
  return;
}

