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

/* Runnable queue */
extern queue_s runnable;

/* Currently running thread/task */
thread_t *curr_thr;
task_t *curr_tsk;

/* The "do" part of sched_do_and_block(...) */
typedef void (*sched_do_fn)(void *args);

/* Scheduling API */
int sched_block(thread_t *thr);
int sched_do_and_block(thread_t *thr, sched_do_fn func, void *args);
void raw_unblock(thread_t *thr, queue_node_s *n);
int sched_unblock(thread_t *thr);
int sched_find(int tid);
void schedule(void);


#endif /* __SCHED_H__ */

