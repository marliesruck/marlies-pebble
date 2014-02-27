/** @file mutex.c
 *
 *  @brief This file implements our mutexes.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck
 *
 *  @bug No known bugs
 */

#include "qentry.h"

#include <mutex.h>
#include <mutex_type.h>

#include <assert.h>
#include <syscall.h>
#include <spin.h>


/** @brief Initialize a mutex.
 *
 *  The mutex is initialy set to the unlocked state.
 *
 *  @param mp The mutex to initialize.
 *
 *  @return 0 on success, or a negative integer error code on failure.
 **/
int mutex_init(mutex_t *mp)
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
void mutex_destroy(mutex_t *mp)
{
  assert(mp);
  assert(queue_empty(&mp->queue));
  assert(mp->state != MUTEX_LOCKED);
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
void mutex_lock(mutex_t *mp)
{
  queue_node_s n;
  qentry_s qe;

  assert(mp);

  /* Wait your turn to access the waiting list */
  spin_lock(&mp->lock);

  /* You're not the only one waiting */
  if (mp->state == MUTEX_LOCKED)
  {
    /* Add yourself to the queue */
    qe.tid = gettid();
    queue_init_node(&n, &qe);
    queue_enqueue(&mp->queue, &n);

    /* Unlock and deschedule */
    qe.reject = 0;
    spin_unlock(&mp->lock);
    while (!qe.reject)
      deschedule(&qe.reject);

    /* Clean-up your cll node */
    cll_final_node(&n);
  }

  /* It's all yours, buddy */
  else {
    mp->state = MUTEX_LOCKED;
    mp->owner = gettid();
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
void mutex_unlock(mutex_t *mp)
{
  queue_node_s *n;
  qentry_s *qe;
  int tid;

  assert(mp);

  /* Wait your turn to access the waiting list */
  spin_lock(&mp->lock);

  /* You can only unlock mutexes that you locked */
  assert(mp->state == MUTEX_LOCKED);
  assert(mp->owner == gettid());

  /* There's someone waiting on you */
  if (!cll_empty(&mp->queue))
  {
    /* Extract the next guy in the queue */
    n = queue_dequeue(&mp->queue);
    qe = queue_entry(qentry_s *, n);

    /* Grab the his TID; set the mutex owner.
     *
     * We grab the TID in case the write to his reject wakes him up and he
     * tramples his stack, invalidating his queue entry and it's TID field.
     */
    tid = qe->tid;
    mp->owner = tid;

    /* Unlock and awaken that guy */
    spin_unlock(&mp->lock);
    qe->reject = gettid();
    make_runnable(tid);
  }

  /* No one wants the lock */
  else {
    mp->owner = -1;
    mp->state = MUTEX_UNLOCKED;
    spin_unlock(&mp->lock);
  }

  return;
}

