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
#include <cllist.h>
#include <dispatch.h>

/* Libc specific includes */
#include <assert.h>
#include <malloc.h>

/* x86 includes */
#include <asm.h>

/** @var runnable
 *  @brief The list of runnable threads.
 **/
cll_list runnable = CLL_LIST_INITIALIZER(runnable);


/*************************************************************************
 *  Runqueue manipulation
 *************************************************************************/

/** @brief Retrieve the head of the run queue.
 *
 *  @return The head of the runqueue.
 **/
#define RQ_HEAD   \
  ( (thread_t *)runnable.next->data )

/** @brief Make a thread eligible for CPU time.
 *
 *  @param n The node to add to the runnable queue.
 *  @param thr The thread to make runnable.
 *
 *  @return Void. 
 **/
void rq_add(thread_t *thr)
{
  assert(thr->state != THR_RUNNABLE);
  cll_insert(runnable.next, &thr->rq_entry);
  thr->state = THR_RUNNABLE;

  return;
}

/** @brief Make a thread ineligible for CPU time.
 *
 *  @param thr The thread to make unrunnable.
 *
 *  @return Void.
 **/
void rq_del(thread_t *thr)
{
  assert(thr->state == THR_RUNNABLE);
  assert(cll_extract(&runnable, &thr->rq_entry));
  thr->state = THR_BLOCKED;
  
  return;
}

/** @brief Move a runnable thread to the back of the run queue.
 *
 *  @return 0 on success, or a negative integer error code on failure.
 **/
int rq_rotate(thread_t *thr)
{
  /* Return NULL for an empty run queue */
  if (queue_empty(&runnable)) return -1;

  /* Move the head to the back of the queue */
  assert(thr->state == THR_RUNNABLE);
  assert(cll_extract(&runnable, &thr->rq_entry));
  cll_insert(&runnable, &thr->rq_entry);

  return 0;
}

/** @brief Search the runqueue for a particular TID.
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
    thr = cll_entry(thread_t *, n);
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

  /* Lock, insert, unlock */
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
 *  @return Void.
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
 *  @return Void.
 **/
void sched_block(thread_t *thr)
{
  disable_interrupts();

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

  /* Move the yielder to the back */
  assert( !rq_rotate(curr_thr) );

  /* Dispatch the target (if it's not the yielder) */
  if (thr->tid != curr_thr->tid) dispatch(thr);

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
  next = RQ_HEAD;
  assert( !rq_rotate(next) );

  /* Only switch if the next thread is different */
  if (next->tid != curr_thr->tid)
    dispatch(next);

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

