/* @file rwlock.c
 *
 * @brief Implements readers writer locks
 *
 * @author Marlies Ruck (mruck)
 * @author Enrique Naudon (esn)
 * @bug No known bugs
 */

#include <rwlock.h>
#include <rwlock_type.h>

#include <assert.h>
#include <syscall.h>
#include <spin.h>
#include <malloc.h>
#include "qentry.h"

/******* Helper routines ********/
int broadcast_readers(rwlock_t *rwlock);
void read_lock(rwlock_t *rwlock);
void write_lock(rwlock_t *rwlock);
int read_unlock(rwlock_t *rwlock);
void update_rwlock(rwlock_t *rwlock);
cll_node *dynam_node_init(void);

/* @brief Initalize a reader writer lock
 *
 * @param rwlock Lock to be initialized
 * @return -1 if the lock is invalid(NULL) otherwise 0 on success
 */
int rwlock_init(rwlock_t *rwlock)
{
  if(rwlock == NULL)
    return -1;
  spin_init(&rwlock->lock);
  cll_init_list(&rwlock->writers);
  cll_init_list(&rwlock->readers_waiting);
  cll_init_list(&rwlock->readers_reading);
  rwlock->state = RWLOCK_UNLOCKED;

  return 0;
}

/* @brief "Deactive" a reader writer lock
 *
 * It is illegal for an application to use a reader writers lock after it has
 * been destroyed (unless and until it is later re-initalized).  It is illegal
 * for an application to invoke rwlock_destroy() on a lock while the lock is
 * held or while threads are waiting on it.
 *
 * @param rwlock Lock to be destroyed
 */
void rwlock_destroy(rwlock_t *rwlock)
{
  assert(rwlock != NULL);
  assert(rwlock->state == RWLOCK_UNLOCKED);
  return;
}

/*
 * @brief Blocks the calling thread until it has been granted the requested form
 * of access
 *
 * Relies on helper routines read_lock and write_lock dependening upon the type
 * of access requested.  Assumes those functions release the lock;
 *
 * @param rwlock Lock to be acquired
 * @param type Indicates desired access level, must be either RWLOCK_WRITE (for
 * an exclusive lock) or RWLOCK_READ(for a shared lock) 
 */
void rwlock_lock(rwlock_t *rwlock, int type)
{
  assert(rwlock);

  spin_lock(&rwlock->lock);

  switch(type)
  {
    case RWLOCK_READ:
      read_lock(rwlock);
      break;
    case RWLOCK_WRITE:
      write_lock(rwlock);
      break;
    default:
      spin_unlock(&rwlock->lock);
      break;
   }

  return;
}

/* @brief Indicates that the calling thread is done using the locked state in
 * whichever mode it was granted access for
 *
 * @param rwlock Lock to wake up readers or writers waiting (if any)
 */
void rwlock_unlock(rwlock_t *rwlock) 
{
  assert(rwlock);

  spin_lock(&rwlock->lock);

  /* Validate that the calling thread actually holds the lock */
  int retval;
  switch(rwlock->state)
  {
    case RWLOCK_RDLOCKED:
      /* Calling thread is in current list of readers in critial section or
       * there are still readers in the critical section. Consequently, release
       * the lock and return */
      if(0 > (retval = read_unlock(rwlock))){
        spin_unlock(&rwlock->lock);
        return;
      }
      /* Otherwise, this was the last reader in the critical section and the
       * lock is now free */
      break; 
    case RWLOCK_WRLOCKED:
      /* Ensure the writer calling unlock holds the lock.  If not, return */
      if(gettid() != rwlock->writer_tid){
          spin_unlock(&rwlock->lock);
          return;
      }
      /* Otherwise, the lock is now free, check the queue of waiting threads */
      break;
    default:
      /* You called rwlock_unlock with an invalid type argument */
      spin_unlock(&rwlock->lock);
      return;
  }

  /* The lock is now free.  Let someone else in the critical section */
  update_rwlock(rwlock);
  return;
}

/* @Brief Allows writer holding to lock to downgrade to reader status
 *
 * A thread may call this function only if it already holds the lock in
 * RWLOCK_WRITE mode at a time when it no longer requires exclusive access to
 * the protected resource.  When the function returns no threads hold the hlock
 * in RWLOCK_WRITE mode; the invoking thread, and possible other readers waiting
 * to enter the critical section, hold the lock in RWLOCK_READ mode.  Previously
 * blocked or newly arriving writers must still wait for the lock to be released
 * entirely.  
 *
 * @param rwlock Lock to be downgraded
 */
void rwlock_downgrade(rwlock_t *rwlock)
{
  assert(rwlock);

  spin_lock(&rwlock->lock);

  /* Ensure the calling thread actually own the lock in RWLOCK_WRITE mode */
  if((rwlock->state == RWLOCK_WRLOCKED) && (rwlock->writer_tid == gettid()))
  {
    /* Wake up all readers */
    broadcast_readers(rwlock);

    /* Reset state */
    rwlock->state = RWLOCK_RDLOCKED;

    /* Add downgraded writer to the list of readers in the critical section */
    cll_node *n = dynam_node_init();
    cll_insert(&rwlock->readers_reading,n);
  }
  spin_unlock(&rwlock->lock);
  return;
} 

/****************** Helper routines *********************/

/* @Brief Allow all waiting readers to enter critical section
 *
 * Assumptions:
 * 1) rwlock is locked before being called and lock is released after call
 * 2) rwlock->state is set to RWLOCK_READ by calling function 
 *
 * @param rwlock Lock to wake up readers on
 * @return -1 if there are no readers to wake up, else 0
 */
int broadcast_readers(rwlock_t *rwlock)
{
  /* No readers waiting */
  if(cll_empty(&rwlock->readers_waiting))
    return -1;

  /* Run list of readers waiting */
  cll_node *n;
  cll_foreach(&rwlock->readers_waiting,n){
    qentry_s *reader = cll_entry(qentry_s *, n);
    reader->reject = gettid();
    make_runnable(reader->tid);
  }

  /* Update list of readers in the critical section because readers waiting are
   * now readers readings */
  cll_node *waiting= (cll_node*)(&rwlock->readers_waiting);
  waiting->next->prev = (cll_node*)(&rwlock->readers_reading);
  waiting->prev->next = (cll_node*)(&rwlock->readers_reading);
  rwlock->readers_reading = rwlock->readers_waiting;

  /* Re-initalize list of readers waiting */
  cll_init_list(&rwlock->readers_waiting); 

  return 0;
}

/* @Brief Dynamically initialized a qentry struct
 *
 * Readers who proceed directly to the critical section need a dynamically
 * allocated so that it doesn't get clobbered when they return.  This is
 * important because we need to keep track of the readers in the critical
 * section.  Since the readers_waiting will eventually be the readers_reading we
 * malloc their qentry_s so we can reuse it later instead of statically
 * allocating and then mallocing later.  
 *
 * @return Initialized node
 */
cll_node *dynam_node_init(void)
{
  qentry_s *data = malloc(sizeof(qentry_s));
  cll_node *n = malloc(sizeof(cll_node));

  data->tid = gettid();
  data->reject = 0;
  cll_init_node(n, data);
  return n;
}

/* @Brief Allow reader to directly enter critical section or enqueue dependeing
 * on whether or not writers are waiting/writing
 *
 * Assumes rwlock is locked before the call.  Note the read_lock() releases lock
 * upon return
 *
 * @param rwlock Lock readers are waiting on
 */
void read_lock(rwlock_t *rwlock)
{
  cll_node *n = dynam_node_init();
  qentry_s *data;

  switch(rwlock->state)
  {
    case RWLOCK_UNLOCKED:
      /* Set state and fall through */
      rwlock->state = RWLOCK_RDLOCKED;
    case RWLOCK_RDLOCKED:
      /* If the writer queue is empty proceed to the critical
       * section. */
      if(cll_empty(&rwlock->writers)){
        /* Keep track of readers in the critical section */
        cll_insert(&rwlock->readers_reading, n);
        /* Relinquish lock */
        spin_unlock(&rwlock->lock);
        break;
      }
    /* Writers are waiting/writing, fall through and enqueue yourself */
    case RWLOCK_WRLOCKED:
      cll_insert(&rwlock->readers_waiting,n);
      data = cll_entry(qentry_s *,n);
      spin_unlock(&rwlock->lock);
      do deschedule(&data->reject);
      while (data->reject == 0);
      break;
    default:
      spin_unlock(&rwlock->lock);
      break;
  }
  return;
}

/* @Brief Allow writer to enter critical section or enqueue
 *
 * Assumes rwlock is locked before the call.  Note the write_lock() releases
 * lock upon return
 *
 * @param rwlock Lock readers are waiting on
 */
void write_lock(rwlock_t *rwlock)
{
  /* Lock is free.  Take it. */
  if(rwlock->state == RWLOCK_UNLOCKED){
    rwlock->state = RWLOCK_WRLOCKED;
    rwlock->writer_tid = gettid();
    spin_unlock(&rwlock->lock);
  }
  /* A reader or writer currently holds the lock.  Enqueue yourself */
  else{
    cll_node n;
    qentry_s data;
    data.tid = gettid();
    data.reject = 0;
    cll_init_node(&n,&data);
    cll_insert(&rwlock->writers,&n);

    spin_unlock(&rwlock->lock);
    do deschedule(&data.reject);
    while (data.reject == 0);
  }
}

/* @Brief Decrements the count of readers in the critical section
 *
 * Assumes rwlock is locked before being called and unlocked upon return.
 *
 * @param rwlock Lock to check for list of current readers in the critical
 * section
 * @return -1 if the calling thread is not in the current list of readers in the
 * critical section or if there are still readers in the critical section.
 * Else, 0 if there are no readers left in the critical section and the lock has
 * been relinquished
 */
int read_unlock(rwlock_t *rwlock)
{
  cll_node *n;
  qentry_s *data;
  int tid = gettid();

  /* Ensure reader calling unlock is actually in the critical section */
  cll_foreach(&rwlock->readers_reading,n){
    data = cll_entry(qentry_s *, n);
    if(tid == data->tid) 
      break;
  }

  /* We traversed the entire list of readers and didn't find the tid */
  if((cll_list*)(n) == &rwlock->readers_reading)
    return -1;

  /* Free data and node */
  n = cll_extract(&rwlock->readers_reading, n);
  free(data);
  free(n);


  /* There are still readers in the critical section */
  if(!cll_empty(&rwlock->readers_reading)){
    return -1;
  }

  /* No readers are left. Return and wake up the head of the waiting queue */
  return 0;
}

/* @Brief Lock has been fully relinquished by calling thread.  Update the state
 * of the lock and check for readers/writers in the queue
 *
 * Assumes rwlock is locked before call 
 *
 * @param rwlock Lock to check for waiting readers/writers
 */
void update_rwlock(rwlock_t *rwlock)
{
  /* No new reader can acquire a shared lock if a writer is waiting */
  if(!(cll_empty(&rwlock->writers)))
  {
    cll_node *head = cll_extract(&rwlock->writers,rwlock->writers.next);
    qentry_s *writer = cll_entry(qentry_s *, head);

    /* update lock owner and state */
    rwlock->writer_tid = writer->tid;
    rwlock->state = RWLOCK_WRLOCKED;

    /* Don't be selfish. Let someone else join the queue */
    spin_unlock(&rwlock->lock);

    /* Wake up the writer */
    writer->reject = gettid();
    make_runnable(writer->tid);
  }
  /* No writers are waiting, wake up the readers */
  else if (broadcast_readers(rwlock) >= 0){
    rwlock->state = RWLOCK_RDLOCKED;
    spin_unlock(&rwlock->lock);
  }
  /* Broadcast_readers() returned -1 because there are no readers waiting.  
   * Set the state to unlocked */
  else{
      rwlock->state = RWLOCK_UNLOCKED;
      spin_unlock(&rwlock->lock);
  }
  return;
}

