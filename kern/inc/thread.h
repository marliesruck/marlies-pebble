/* @file thread.h
 *
 * @brief Declares structures and functions for thread manipulation.
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug No known bugs
 */
#ifndef __THREAD_H__
#define __THREAD_H__

/* Pebble includes */
#include <mutex.h>
#include <process.h>
#include <queue.h>
#include <vm.h>

/* Libc includes */
#include <x86/page.h>
#include <stdint.h>

#define KSTACK_SIZE PAGE_SIZE

/** @enum thread_state
 *  @brief Flag values for thread state.
 **/
enum thread_state {
  THR_NASCENT,    /**< The thread is still being initialized **/
  THR_RUNNING,    /**< The thread is currently runnable **/
  THR_BLOCKED,    /**< The thread is currently not runnable **/
  THR_EXITING,    /**< The thread is exiting **/
};
typedef enum thread_state thr_state_e;

struct thread {
  struct task *task_info;
  thr_state_e state;
  mutex_s lock;
  int tid;
  void *sp;
  void *pc;
  char kstack[KSTACK_SIZE];
};
typedef struct thread thread_t;

/* Initialization routines */
thread_t *thread_init(struct task *t);

/* Manipulation routines */
void thrlist_enqueue(thread_t *thread, queue_s *q);
thread_t *thrlist_dequeue(queue_s *q);

/* Thread list manipulation */
int thrlist_add(thread_t *t);
int thrlist_del(thread_t *t);
thread_t *thrlist_find(int tid);

#endif /* _THREAD_H */
