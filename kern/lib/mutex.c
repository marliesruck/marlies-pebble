/** @file mutex.c
 *
 *  @brief This file implements our mutexes.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck
 *
 *  @bug No known bugs
 */

/* Mutex includes */
#include <mutex.h>

/* Pebbles includes */
#include <assert.h>
#include <sched.h>
#include <spin.h>

/* x86 includes */
#include <x86/asm.h>

/** @enum unlock mode

 *  @brief Flags for whether a mutex is being unlocked with interrupts
 *         enabled or disabled
 **/
enum unlock_mode {
  NO_BLOCK,
  BLOCK,
};
typedef enum unlock_mode unlock_mode_e;


/*************************************************************************
 *  Internal helper functions
 *************************************************************************/

/** @brief Internal unlocking function.
 *
 *  There are two unlocking modes, one where we disable interrupts and
 *  schedule after adding the next thread to the runqueue, and one where we
 *  do not schedule and do not disable interrupts.  The point is so that we
 *  can unlock a mutex while interrupts are disabled (i.e. in
 *  sched_do_and_unblock(...)).
 *
 *  @param mp The mutex to unlock.
 *  @param mode Whether or not interrupts are currently enabled or disabled.
 *
 *  @return Void.
 **/
void unlock(mutex_s *mp, unlock_mode_e mode)
{
  queue_node_s *n;
  thread_t *thr;

  assert(mp);

  /* Wait your turn to access the waiting list */
  spin_lock(&mp->lock);

  /* You can only unlock locked mutexes that you locked...or can you? */
  assert(mp->owner == curr_thr->tid);
  assert(mp->state == MUTEX_LOCKED);

  /* There's someone waiting on you */
  if (!cll_empty(&mp->queue))
  {
    /* Extract the next guy in the queue */
    n = queue_dequeue(&mp->queue);
    thr = queue_entry(thread_t *, n);

    /* Set the new owner */
    mp->owner = thr->tid;

    /* Unlock and awaken that guy */
    disable_interrupts();
    spin_unlock(&mp->lock);
    if(mode == BLOCK) rq_del(curr_thr);
    rq_add(thr);
    schedule_unprotected();
    enable_interrupts();
  }

  /* No one wants the lock */
  else {
    mp->owner = -1;
    mp->state = MUTEX_UNLOCKED;
    if (mode == NO_BLOCK) spin_unlock(&mp->lock);
    else spin_unlock_and_block(&mp->lock);
  }

  return;
}


/*************************************************************************
 *  Exported API
 *************************************************************************/

/** @brief Initialize a mutex.
 *
 *  The mutex is initialy set to the unlocked state.
 *
 *  @param mp The mutex to initialize.
 *
 *  @return 0 on success, or a negative integer error code on failure.
 **/
int mutex_init(mutex_s *mp)
{
  if(mp == NULL) return -1;

  spin_init(&mp->lock);
  mp->state = MUTEX_UNLOCKED;
  mp->owner = -1;

  queue_init(&mp->queue);

  return 0;
}

/** @brief Finalize a mutex.
 *
 *  @param mp The mutex to finalize.
 *
 *  @return Void.
 **/
void mutex_final(mutex_s *mp)
{
  assert(mp);
  assert(queue_empty(&mp->queue));
  assert(mp->state != MUTEX_LOCKED);
  assert(mp->owner == -1);
  return;
}

/** @brief Lock a mutex.
 *
 *  If the lock is not avaiable, mutex_lock(...) will place the calling
 *  task on a wait-queue and deschedule it--that is, this function may
 *  block.
 *
 *  @param mp The mutex to lock.
 *
 *  @return Void.
 **/
void mutex_lock(mutex_s *mp)
{
  queue_node_s n;

  assert(mp);

  /* Wait your turn to access the waiting list */
  spin_lock(&mp->lock);

  /* You're not the only one waiting */
  if (mp->state == MUTEX_LOCKED)
  {
    /* Add yourself to the queue */
    queue_init_node(&n, curr_thr);
    queue_enqueue(&mp->queue, &n);

    /* Unlock-and-block and deschedule */
    spin_unlock_and_block(&mp->lock);

    /* Clean-up your cll node */
    cll_final_node(&n);
  }

  /* It's all yours, buddy */
  else {
    mp->state = MUTEX_LOCKED;
    mp->owner = curr_thr->tid;
    spin_unlock(&mp->lock);
  }

  return;
}

/** @brief Unlock a mutex.
 *
 *  @param mp The mutex to unlock.
 *
 *  @return Void.
 **/
void mutex_unlock(mutex_s *mp)
{
  unlock(mp, NO_BLOCK);
  return;
}

/** @brief Atomically unlock a mutex and block the caller.
 *
 *  @param mp The mutex to unlock.
 *
 *  @return Void.
 **/
void mutex_unlock_and_block(mutex_s *mp)
{
  unlock(mp, BLOCK);
  return;
}

