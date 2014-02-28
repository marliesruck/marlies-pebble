/** @file mutex_type.h
 *
 *  @brief This file defines the type for mutexes.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck
 *
 *  @bug No known bugs
 */

#ifndef __MUTEX_TYPE_H__
#define __MUTEX_TYPE_H__

#include <spin.h>
#include <queue.h>


/** @brief Flag values for mutex state.
 **/
enum mutex_state {
  MUTEX_UNLOCKED,
  MUTEX_LOCKED,
};
typedef enum mutex_state mutex_state_e;

/** @brief A heavier-weight possibly-blocking lock.
 **/
struct mutex {
  spin_s lock;
  mutex_state_e state;
  queue_s queue;
  int owner;
};
typedef struct mutex mutex_t;

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


#endif /* __MUTEX_TYPE_H__ */

