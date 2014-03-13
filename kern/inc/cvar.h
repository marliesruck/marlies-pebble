/** @file cond_type.h
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

struct cvar {
	spin_s lock;
	queue_s queue;
};
typedef struct cvar cvar_s;

/* condition variable functions */
int cond_init( cvar_s *cv );
void cond_final( cvar_s *cv );
void cond_wait( cvar_s *cv, mutex_s *mp );
void cond_signal( cvar_s *cv );
void cond_broadcast( cvar_s *cv );

#endif /* __COND_H__ */
