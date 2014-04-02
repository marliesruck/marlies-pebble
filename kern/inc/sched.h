/** @file sched.h
 *
 *  @brief Declares our scheduler's API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 */
#ifndef __SCHED_H__
#define __SCHED_H__

#include <mutex.h>
#include <queue.h>
#include <spin.h>
#include <thread.h>

/* Scheduler lock */
extern mutex_s sched_lock;

extern queue_s runnable;

/* The currently running thread */
thread_t *curr;

/* Scheduling API */
int sched_block(thread_t *thr);
int sched_mutex_unlock_and_block(thread_t *thr, mutex_s *lock);
int sched_spin_unlock_and_block(thread_t *thr, spin_s *lock);
void raw_unblock(thread_t *thr, queue_node_s *n);
int sched_unblock(thread_t *thr);
int sched_find(int tid);
void schedule(void);


#endif /* __SCHED_H__ */

