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


extern queue_s runnable;

void schedule(void);
int sched_block(int tid);
int sched_unblock(int tid);


#endif /* __SCHED_H__ */
