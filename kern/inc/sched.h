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

#include <queue.h>
#include <thread.h>


/* The currently running thread */
thread_t *curr;

/* Scheduling API */
int sched_block(thread_t *thr);
int sched_unblock(thread_t *thr);
void schedule(void);


#endif /* __SCHED_H__ */

