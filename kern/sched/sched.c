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

  /* The node is embedded in the thread struct so don't free */
  thr->state = THR_BLOCKED;
  
  schedule();

  return 0;
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
  cll_node *n;
  thread_t *thr;

  /* Ensure the favored thread is eligible for CPU time */
  disable_interrupts();
  cll_foreach(&runnable, n) {
    thr = queue_entry(thread_t *, n);
    if (thr->tid == tid){
      schedule();
      enable_interrupts();
      return 0;
    }
  }

  /* Favored thread is currently ineligible for CPU times */
  enable_interrupts();
  return -1;
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

/** @brief Atomically execute a caller-specified function and block.
 *
 *  @param thr The thread to make unrunnable.
 *  @param fn The function to execute before blocking.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int sched_do_and_block(thread_t *thr, sched_do_fn func, void *args)
{
  int ret;

  assert(thr);

  /* Lock the run queue, unlock the world lock */
  disable_interrupts();

  func(args);

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
  /* Don't malloc a node, instead use the embedded list traversal structure */

  /* Lock, enqueue, unlock */
  disable_interrupts();
  raw_unblock(thr,&thr->node);
  schedule();
  enable_interrupts();

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
  queue_enqueue(&runnable, q);

  /* Only switch if the next thread is different */
  if (next != curr_thr) {
    dispatch_wrapper(next);
  }

  return;
}

