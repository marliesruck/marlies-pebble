/** @file cvar.h
 *
 *  @brief This file defines the type for condition variables.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 */
#ifndef __COND_H__
#define __COND_H__

#include <mutex.h>
#include <queue.h>
#include <spin.h>

/** @struct cvar
 *  @brief A condition variable.
 **/
struct cvar {
	spin_s lock;    /**< Protects the cvar struct fields **/
	queue_s queue;  /**< Queue of waiting threads **/
};
typedef struct cvar cvar_s;

/** @brief Statically initialize a condition variable.
 *
 *  @return A statically initialized cvar.
 **/
#define CVAR_INITIALIZER(name) {          \
  .lock = SPIN_INITIALIZER(),             \
  .queue = QUEUE_INITIALIZER(name.queue)  \
}

/* Condition variable operations */
int cvar_init( cvar_s *cv );
void cvar_final( cvar_s *cv );
void cvar_wait( cvar_s *cv, mutex_s *mp );
void cvar_signal( cvar_s *cv );
void cvar_signal_raw( cvar_s *cv );
void cvar_broadcast( cvar_s *cv );

#endif /* __COND_H__ */
