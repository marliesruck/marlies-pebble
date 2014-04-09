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


/* The "do" part of sched_do_and_block(...) */
typedef void (*sched_do_fn)(void *args);

/* Currently running thread/task */
thread_t *curr_thr;
task_t *curr_tsk;

/* Scheduling API */
void sched_unblock(thread_t *thr);
void sched_block(thread_t *thr);
void sched_do_and_block(thread_t *thr, sched_do_fn func, void *args);
int sched_find(int tid);
void schedule(void);
void schedule_unprotected(void);
int sched_add_to_rq(thread_t *thr);

/* Runqueue manipulation */
void rq_add(thread_t *thr);
void rq_del(thread_t *thr);
thread_t *rq_rotate(void);
thread_t *rq_find(int tid);


#endif /* __SCHED_H__ */

