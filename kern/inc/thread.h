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
#include <cllist.h>
#include <mutex.h>
#include <process.h>
#include <queue.h>
#include <sc_utils.h>
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
  cll_node node; /* Embedded list traversal struct for the runnable queue */
  cll_node zzz_node; /* Embedded list traversal struct for sleeping */
  int killed;    /* Flag to indicate if thread was killed by kernel */
  thr_state_e state;
  mutex_s lock;
  int tid;
  void *sp;
  void *pc;
  swexn_t swexn;
  char kstack[KSTACK_SIZE];
};
typedef struct thread thread_t;

/* Initialization routines */
thread_t *thread_init(struct task *t);
int thr_launch(thread_t *t, void *sp, void *pc);

/* Thread list manipulation */
int thrlist_add(thread_t *t);
int thrlist_del(thread_t *t);
thread_t *thrlist_find(int tid);

#endif /* __THREAD_H__ */

