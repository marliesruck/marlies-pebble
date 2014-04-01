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

mutex_s sched_lock = MUTEX_INITIALIZER(sched_lock);

/** @brief Make a thread ineligible for CPU time.
 *
 *  In this function we use the linked list functions instead of the queue
 *  interface; this is because the queue API does not allow searching or
 *  extraction of arbitrary nodes.
 * 
 *  @param thr The thread to make unrunnable.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int raw_block(thread_t *thr)
{
  cll_node *n;

  /* Ensure the blockee is runnable */
  cll_foreach(&runnable, n) {
    if (thr == queue_entry(thread_t *, n)) break;
  }
  if (thr != queue_entry(thread_t *, n)) return -1;

  /* Remove the blockee node from the runnable queue */
  assert(cll_extract(&runnable, n) == n);
  free(n);
  thr->state = THR_BLOCKED;
  
  schedule();

  return 0;
}

/** @brief Make a thread ineligible for CPU time.
 *
 *  @param thr The thread to make unrunnable.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int sched_block(thread_t *thr)
{
  int ret;

  /* Lock, block, unlock */
  disable_interrupts();
  ret = raw_block(thr);
  assert(ret == 0);
  enable_interrupts();

  return ret;
}

/** @brief Atomically unlock and make a thread ineligible for CPU time.
 *
 *  If the lock paramter is non-NULL, it will be unlocked before blocking.
 *  The process of unlocking and blocking is atomic with respect to other
 *  atempts to block/unblock, and with respect to attempts to schedule
 *  another process.
 * 
 *  @param thr The thread to make unrunnable.
 *  @param lock The spinlock to unlock.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int sched_spin_unlock_and_block(thread_t *thr, spin_s *lock)
{
  int ret;

  assert(thr);

  /* Lock the run queue, unlock the world lock */
  disable_interrupts();

  if (lock) spin_unlock(lock);

  ret = raw_block(thr);
  assert(ret == 0);

  /* Unlock and return */
  enable_interrupts();
  return ret;
}

/** @brief Atomically unlock and make a thread ineligible for CPU time.
 *
 *  If the lock paramter is non-NULL, it will be unlocked before blocking.
 *  The process of unlocking and blocking is atomic with respect to other
 *  atempts to block/unblock, and with respect to attempts to schedule
 *  another process.
 *
 *  @param thr The thread to make unrunnable.
 *  @param lock The mutex to unlock.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int sched_mutex_unlock_and_block(thread_t *thr, mutex_s *lock)
{
  int ret;

  /* Lock the run queue, unlock the world lock */
  disable_interrupts();
  if (lock) mutex_unlock(lock);

  ret = raw_block(thr);
  assert(ret == 0);

  /* Unlock and return */
  enable_interrupts();
  return ret;
}
/** @brief Make a thread eligible for CPU time.
 *
 *  This operation is not atomic.
 *
 *  @param n The node to add to the runnable queue.
 *  @param thr The thread to make runnable.
 *
 *  @return Void. 
 **/
void raw_unblock(thread_t *thr, queue_node_s *n)
{
  queue_enqueue(&runnable, n);
  thr->state = THR_RUNNING;

  return;
}

/** @brief Make a thread eligible for CPU time.
 *
 *  This operation is atomic.
 *
 *  @param thr The thread to make runnable.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int sched_unblock(thread_t *thr)
{
  queue_node_s *n;

  /* Allocate and init a queue node */
  n = malloc(sizeof(cll_node));
  if (!n) return -1;
  queue_init_node(n, thr);

  /* Lock, enqueue, unlock */
  disable_interrupts();
  raw_unblock(thr,n);
  schedule();
  enable_interrupts();

  return 0;
}
/* @brief Remove yourself from runnable queue.
 * 
 * Assumes scheduler is locked.
 *
 * @param thr Thread to remove.
 * @return -1 on error, 0 otherwise.
 */
int remove_from_runnable(thread_t *thr)
{
  cll_node *n;

  /* Ensure thread is runnable */
  cll_foreach(&runnable, n) {
    if (thr == queue_entry(thread_t *, n)) break;
  }
  if (thr != queue_entry(thread_t *, n)) return -1;

  /* Remove the thread node from the runnable queue */
  assert(cll_extract(&runnable, n) == n);
  free(n);

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
  thread_t *next;
  queue_node_s *q;

  /* The runnable queue should never be empty */
  assert( !queue_empty(&runnable) );

  /* Move the lucky thread to the back of the queue */
  q = queue_dequeue(&runnable);
  next = queue_entry(thread_t *, q);
  assert((next->state == THR_RUNNING) || (next->state == THR_EXITING));
  queue_enqueue(&runnable, q);

  /* Only switch if the next thread is different */
  if (next != curr) {
    dispatch_wrapper(next);
  }

  return;
}

