/** @files sched.c
 *
 *  @brief Implements our scheduler.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <sched.h>

#include <ctx_switch.h>
#include <spin.h>

#include <assert.h>
#include <malloc.h>
#include <simics.h>


/** @var rq_lock
 *  @brief A lock for the run queue.
 *
 *  The decision to protect the runnable queue with a spinlock was a
 *  deliberate one.  We cannot use a mutex here because our mutex
 *  implementation relies on the scheduler to block waiting threads.  We
 *  could disable interrupts, but that would not port well to an SMP
 *  system, and various lab handouts have noted that previous P4s have been
 *  implementing SMP support.
 **/
spin_s rq_lock = SPIN_INITIALIZER();

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
int sched_block(thread_t *thr)
{
  cll_node *n;

  /* Lock the run queue */
  spin_lock(&rq_lock);

  /* Ensure the blockee is runnable */
  cll_foreach(&runnable, n) {
    if (thr == queue_entry(thread_t *, n)) break;
  }
  if (thr != queue_entry(thread_t *, n)) return -1;

  /* Remove the blockee node from the runnable queue */
  assert(cll_extract(&runnable, n) == n);
  free(n);
  thr->state = THR_BLOCKED;

  spin_unlock(&rq_lock);
  return 0;
}

/** @brief Make a thread eligible for CPU time.
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
  spin_lock(&rq_lock);
  queue_enqueue(&runnable, n);
  thr->state = THR_RUNNING;
  spin_unlock(&rq_lock);
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

  /* The runnable queue should never be empty */
  assert( !queue_empty(&runnable) );

  /* Move the lucky thread to the back of the queue */
  spin_lock(&rq_lock);
  q = queue_dequeue(&runnable);
  next = queue_entry(thread_t *, q);
  assert(next->state == THR_RUNNING);
  queue_enqueue(&runnable, q);
  spin_unlock(&rq_lock);

  /* Only switch if the next thread is different */
  if (next != curr) {
    prev = curr;
    curr = next;
    ctx_switch(&prev->sp, &prev->pc, next->sp, next->pc,
               next->task_info->cr3, &next->kstack[KSTACK_SIZE]);
  }

  return;
}

