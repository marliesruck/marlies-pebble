/** @file cvar.c
 *
 *  @brief This file implements our condition variables.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No knowns bugs
 **/

#include "qentry.h"

#include <cond.h>

#include <assert.h>
#include <spin.h>
#include <mutex.h>
#include <syscall.h>
#include <thread.h>

/* @brief Initialize cond var 
 * 
 *  The effects of using a condition variable before it has been initalized or
 *  of initializing it when it is already initalized and in use are undefined.
 *
 *  Although statically allocating the cond var's mutex would be preferable, it
 *  would be stored on the thread's stack and there is a possibility that thread
 *  may exit while other threads are using the cond var and its stack would be
 *  unmapped.  Consequently, we malloc.
 *
 * @param cv Cond var to initialize.  
 * @return -1 if cv is invalid(NULL) else 0 on success
 */
int cond_init(cond_t *cv)
{
  if(cv == NULL) return -1;

  spin_init(&cv->lock);
  queue_init(&cv->queue);

  return 0;
}
/* @brief "deactive" cond var
 *
 * It is illegal for an application to use a condition variable after it has
 * been destroyed(unless and until it is later re-initialized).  It is illegal
 * for an application to invoke cond_destroy() on a condition variable while
 * threads are blocked waiting on it.
 *
 * @param cv Cond var to destroy
 */
void cond_destroy(cond_t *cv)
{
  assert(cv);
  assert(queue_empty(&cv->queue));
  return;
}

/* @brief Allows a thread to wait for a condition and release the associated
 * mutex that it needs to hold to check that condition 
 *        
 * The calling thread blocks, waiting to be signaled.  The blocked thread
 * may be awakended by a cond_signal() or cond_broadcast().  
 *
 * Statically allocate a queue_node to add to the queue of threads waiting to
 * be signaled.  This struct can be statically allocated because the thread
 * is de-scheduled while waiting to be signaled so its stack will remain
 * intact.
 *
 * @param *cv Condition variable 
 * @param *mp Mutex needed to hold to check the condition.  Upon return from 
 * cond_wait() *mp had been re-acquired on behalf of the calling thread
 */
void cond_wait(cond_t *cv, mutex_t *mp)
{
  queue_node_s n;
  qentry_s qe;

  assert(cv);
  assert(mp);

  qe.tid = gettid();

  /* lock cvar mutex and atomically enqueue */
  queue_init_node(&n, &qe);
  spin_lock(&cv->lock);
  queue_enqueue(&cv->queue, &n);

  /* Release world mutex */
  mutex_unlock(mp);

  /* Unlock and deschedule */
  qe.reject = 0;
  spin_unlock(&cv->lock);
  while (qe.reject == 0)
    deschedule(&(qe.reject));

  /* Lock world mutex and make progress */
  mutex_lock(mp);
  return;
}
/* @brief Wake up a thread waiting on the condition variable
 *
 * @param cv Condition variable with queue of threads to awaken
 */
void cond_signal(cond_t *cv) 
{
  queue_node_s *n;
  qentry_s *qe;

  assert(cv);

  /* Atomically access condition variable */
  spin_lock(&cv->lock);

  /* If someone is in the queue, signal them */
  if (!queue_empty(&cv->queue)) {
    n = queue_dequeue(&cv->queue);
    qe = queue_entry(qentry_s *, n);

    /* Unlock and wake */
    int tid = qe->tid;
    spin_unlock(&cv->lock);
    qe->reject = 1; 
    make_runnable(tid);
  }

  /* Otherwise, just unlock */
  else spin_unlock(&cv->lock);

  return;
}

/* @brief Wake up all threads waiting on the condition variable
 *
 * Note: cond_broadcast() does not awaken threads which may invoke cond_wait(cv)
 * "after" this call to cond_broadcast() has begun.
 *
 * @param cv Wake up threads waiting on condition variable pointed to be cv
 */
void cond_broadcast(cond_t *cv) 
{
  queue_node_s *n;
  qentry_s *qe;

  assert(cv);

  spin_lock(&cv->lock);

  /* if someone is in the queue signal them */
  while (!queue_empty(&cv->queue)) {
    n = queue_dequeue(&cv->queue);
    qe = queue_entry(qentry_s *, n);

    qe->reject = 1; 
    make_runnable(qe->tid);
  }

  spin_unlock(&cv->lock);
  return;
}

