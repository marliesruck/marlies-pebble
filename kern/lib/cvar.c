/** @file cvar.c
 *
 *  @brief This file implements our condition variables.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No knowns bugs
 **/
#include <simics.h>

#include <cvar.h>

#include <assert.h>
#include <sched.h>
#include <spin.h>
#include <thread.h>

/** @enum unlock mode
 *  @brief Flags for whether a mutex is being unlocked with interrupts
 *         enabled or disabled
 **/
enum signal_mode {
  ENABLED,
  DISABLED,
};
typedef enum signal_mode signal_mode_e;

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
int cvar_init(cvar_s *cv)
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
 * for an application to invoke cvar_final() on a condition variable while
 * threads are blocked waiting on it.
 *
 * @param cv Cond var to finalize.
 */
void cvar_final(cvar_s *cv)
{
  assert(cv);
  assert(queue_empty(&cv->queue));
  return;
}

/* @brief Allows a thread to wait for a condition and release the associated
 * mutex that it needs to hold to check that condition 
 *        
 * The calling thread blocks, waiting to be signaled.  The blocked thread
 * may be awakended by a cvar_signal() or cvar_broadcast().  
 *
 * Statically allocate a queue_node to add to the queue of threads waiting to
 * be signaled.  This struct can be statically allocated because the thread
 * is de-scheduled while waiting to be signaled so its stack will remain
 * intact.
 *
 * @param *cv Condition variable 
 * @param *mp Mutex needed to hold to check the condition.  Upon return from 
 * cvar_wait() *mp had been re-acquired on behalf of the calling thread
 */
void cvar_wait(cvar_s *cv, mutex_s *mp)
{
  queue_node_s n;

  assert(cv);

  /* lock cvar mutex and atomically enqueue */
  queue_init_node(&n, curr_thr);
  spin_lock(&cv->lock);
  queue_enqueue(&cv->queue, &n);

  /* Release world mutex, unlock-and-block and deschedule */
  if (mp) mutex_unlock(mp);

  sched_do_and_block(curr_thr, (sched_do_fn) spin_unlock, &cv->lock);

  /* Lock world mutex and make progress */
  if (mp) mutex_lock(mp);

  return;
}

/* @brief Wake up a thread waiting on the condition variable
 *
 * @param cv Condition variable with queue of threads to awaken
 */
void cvar_signal_internal(cvar_s *cv, signal_mode_e mode) 
{
  queue_node_s *n;
  thread_t *thr;

  assert(cv);

  /* Atomically access condition variable */
  spin_lock(&cv->lock);

  /* If someone is in the queue, signal them */
  if (!queue_empty(&cv->queue)) {
    n = queue_dequeue(&cv->queue);
    thr = queue_entry(thread_t *, n);

    /* Unlock and wake */
    spin_unlock(&cv->lock);

    /* Interrupts are enabled */
    if(mode == ENABLED){
      assert(!sched_unblock(thr));
    }
    /* Interrupts are already disabled */
    else{
      raw_unblock(thr, &thr->node);
    }
  }

  /* Otherwise, just unlock */
  else spin_unlock(&cv->lock);

  return;
}

/* @brief Wake up all threads waiting on the condition variable
 *
 * Note: cvar_broadcast() does not awaken threads which may invoke cvar_wait(cv)
 * "after" this call to cvar_broadcast() has begun.
 *
 * @param cv Wake up threads waiting on condition variable pointed to be cv
 */
void cvar_broadcast(cvar_s *cv) 
{
  queue_node_s *n;
  thread_t *thr;

  assert(cv);

  spin_lock(&cv->lock);

  /* if someone is in the queue signal them */
  while (!queue_empty(&cv->queue)) {
    n = queue_dequeue(&cv->queue);
    thr = queue_entry(thread_t *, n);

    /* Add t0 the runnable queue */
    sched_add_to_rq(thr);
  }

  spin_unlock(&cv->lock);
  schedule();
  return;
}

void cvar_signal(cvar_s *cv)
{
  cvar_signal_internal(cv, ENABLED);
  return;
}
void cvar_signal_raw(cvar_s *cv)
{
  cvar_signal_internal(cv, DISABLED);
  return;
}
