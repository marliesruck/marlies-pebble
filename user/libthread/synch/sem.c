/** @file sem.c
 *
 *  @brief This file implements semaphores.
 * 
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 **/

#include "qentry.h"

#include <sem_type.h>

#include <assert.h>
#include <spin.h>
#include <syscall.h>
#include <thread.h>

/* @brief Initialize semaphore
 *
 * @param sem Semaphore to be initialized
 * @param count Value to be initalized to 
 * @return -1 if cv is invalid(NULL) or count < 0 else 0 on success
 */
int sem_init(sem_t *sem, int count)
{
  if((sem == NULL) || (count < 0))
    return -1;

  spin_init(&sem->lock);
  queue_init(&sem->queue);

  sem->count = count;

  return 0;
}
/* @brief "deactivate" semaphore
 *
 * Effects of using a semaphore before it has been initalized are undefined.  
 * It is illegal for an application to use a sempahore after it has been
 * destroyed (unless and until it is later re-initialized).  It is illegal for
 * an application to invoke sem_destroy() on a semphaore while threads are
 * waiting on it.
 *
 * @param sem Semaphore to destroy
 */
void sem_destroy(sem_t *sem)
{
  assert(sem);
  assert(queue_empty(&sem->queue)); 
}

/* @brief Allows thread to decrement semaphore value, and may cause it to block
 * indefinitely until it is legal to perform the decrement
 *
 * @param sem Semaphore to decrement
 */
void sem_wait(sem_t *sem)
{
  assert(sem);

  /* Wait your turn to access the global count */
  spin_lock(&sem->lock);

  if(sem->count > 0){
    --(sem->count);
    spin_unlock(&sem->lock);
  }
  else{
    queue_node_s n;
    qentry_s qe;

    /* Enqueue yourself */
    qe.tid = gettid();
    queue_init_node(&n, &qe);
    queue_enqueue(&sem->queue, &n);

    /* Unlock and deschedule */
    qe.reject = 0;
    spin_unlock(&sem->lock);
    while(!qe.reject)
      deschedule(&qe.reject);
  }
}

/* @brief Wake up a thread waiting on the semaphore
 *
 * @param sem semaphore thread is waiting on
 */
void sem_signal(sem_t *sem)
{
  assert(sem);

  /* Wait your turn to access the global count */
  spin_lock(&sem->lock);

  if(!(queue_empty(&sem->queue))) {
    /* Extract the head of the queue */
    queue_node_s *n = queue_dequeue(&sem->queue);
    qentry_s *qe = queue_entry(qentry_s *, n);

    /* Unlock and make runnable head */
    int tid = qe->tid;
    spin_unlock(&sem->lock);
    qe->reject = gettid();
    make_runnable(tid);
  }
  else {
    ++sem->count;
    spin_unlock(&sem->lock);
  }
}

