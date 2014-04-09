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


/** @enum signal_mode
 *  @brief Indicates the state of interrupts when signal is called
 **/
enum signal_mode {
  ENABLED,
  DISABLED,
};
typedef enum signal_mode signal_mode_e;


/*************************************************************************
 *  Internal helper functions
 *************************************************************************/

/** @brief Wake up a thread waiting on the condition variable
 *
 *  There are two signaling modes, one where we disable interrupts and
 *  schedule after adding the wake-ee to the runqueue, and one where we do
 *  not schedule and do not disable interrupts.  The point is so that we can
 *  signal a cvar while interrupts are disabled (i.e. in an interrupt
 *  handler).
 *
 *  @param cv Condition variable with queue of threads to awaken
 *  @param mode Indicates weather or not to disable interrupts for
 *  scheduling
 *
 *  @return Void.
 **/
void signal(cvar_s *cv, signal_mode_e mode) 
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
    if(mode == ENABLED) sched_unblock(thr);
    else rq_add(thr);
  }

  /* Otherwise, just unlock */
  else spin_unlock(&cv->lock);

  return;
}


/*************************************************************************
 *  Exported API
 *************************************************************************/

/** @brief Initialize a condition variable.
 * 
 *  The effects of using a condition variable before it has been initalized or
 *  of initializing it when it is already initalized and in use are undefined.
 *
 *  @param cv The condition variable to initialize. 
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int cvar_init(cvar_s *cv)
{
  if(cv == NULL) return -1;

  spin_init(&cv->lock);
  queue_init(&cv->queue);

  return 0;
}

/** @brief Destroy a condition variable.
 * 
 *  It is illegal for an application to use a condition variable after it has
 *  been destroyed(unless and until it is later re-initialized).  It is illegal
 *  for an application to invoke cvar_final() on a condition variable while
 *  threads are blocked waiting on it.
 * 
 *  @param cv Cond var to finalize.
 * 
 *  @return Void.
 **/
void cvar_final(cvar_s *cv)
{
  assert(cv);
  assert(queue_empty(&cv->queue));
  return;
}

/** @brief Wait on a condition variable.
 *
 *  The calling thread blocks, waiting to be signaled, and will only be
 *  awoken by a call to cvar_signal(...) or cvar_broadcast(...) on the same
 *  condition variable.  The caller may pass NULL for mp; in this case,
 *  cvar_wait(...) will not unlock/lock the mutex.
 *
 *  Statically allocate a queue_node to add to the queue of threads waiting to
 *  be signaled.  This struct can be statically allocated because the thread
 *  is de-scheduled while waiting to be signaled so its stack will remain
 *  intact.
 *
 *  @param *cv The condition variable to wait on.
 *  @param *mp The "world mutex" to drop while waiting.
 *
 *  @return Void.
 **/
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

/** @brief Signal a condition variable.
 *
 *  This signaling function makes protected scheduler calls.  (See
 *  signal(...) for more information.)
 *
 *  @param cv The condition variable to singal.
 *
 *  @return Void.
 **/
void cvar_signal(cvar_s *cv)
{
  signal(cv, ENABLED);
  return;
}

/** @brief Signal a condition variable.
 *
 *  This signaling function makes unprotected scheduler calls.  (See
 *  signal(...) for more information.)
 *
 *  @param cv The condition variable to singal.
 *
 *  @return Void.
 **/
void cvar_signal_raw(cvar_s *cv)
{
  signal(cv, DISABLED);
  return;
}

/** @brief Broadcast to a condition variable.
 *
 *  @param cv The condition variable to broadcast
 *
 *  @return Void.
 **/
void cvar_broadcast(cvar_s *cv) 
{
  queue_node_s *n;
  thread_t *thr;

  assert(cv);

  spin_lock(&cv->lock);

  /* Unblock everyone in the queue (but don't schedule yet) */
  while (!queue_empty(&cv->queue)) {
    n = queue_dequeue(&cv->queue);
    thr = queue_entry(thread_t *, n);
    sched_add_to_rq(thr);
  }

  /* Unlock the spin lock BEFORE scheduling */
  spin_unlock(&cv->lock);
  schedule();
  return;
}

