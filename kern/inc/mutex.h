/** @file mutex.h
 *
 *  @brief This file defines the type for mutexes.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck
 *
 *  @bug No known bugs
 */

#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <spin.h>
#include <queue.h>


/** @enum mutex_state
 *  @brief Flag values for mutex state.
 **/
enum mutex_state {
  MUTEX_UNLOCKED,     /**< None is currently holding the mutex **/
  MUTEX_LOCKED,       /**< The mutex is currently unavailable **/
};
typedef enum mutex_state mutex_state_e;

/** @struct mutex
 *  @brief A heavier-weight possibly-blocking lock.
 **/
struct mutex {
  spin_s lock;
  mutex_state_e state;
  queue_s queue;
  int owner;
};
typedef struct mutex mutex_s;

/** @brief Statically initialize a mutex.
 *
 *  NOTE: This should go in mutex.h, but we cannot edit that file.
 *
 *  @return A statically initialized mutex.
 **/
#define MUTEX_INITIALIZER(name) {           \
  .lock = SPIN_INITIALIZER(),               \
  .state = MUTEX_UNLOCKED,                  \
  .queue = QUEUE_INITIALIZER(name.queue),   \
  .owner = -1                               \
}

/* Mutex operations */
int mutex_init(mutex_s *mp);
void mutex_final(mutex_s *mp);
void mutex_lock(mutex_s *mp);
void mutex_unlock(mutex_s *mp);

#endif /* __MUTEX_H__ */

