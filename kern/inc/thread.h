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


#define KSTACK_SIZE   PAGE_SIZE
#define KSTACK_ALIGN  4

/** @enum thread_state
 *  @brief Flag values for thread state.
 **/
enum thread_state {
  THR_NASCENT,    /**< The thread is still being initialized **/
  THR_RUNNABLE,   /**< The thread is currently runnable **/
  THR_BLOCKED,    /**< The thread is currently not runnable **/
  THR_EXITING,    /**< The thread is exiting **/
};
typedef enum thread_state thr_state_e;

/** @enum thread_deschedule
 *  @brief Flag indicating whether the thread was descheduled by a call to
 *  sys_deschedule(...)
 **/
enum thread_deschedule {
  THR_DESCHED,
  THR_NOT_DESCHED,
};
typedef enum thread_deschedule thr_desched_e;

struct thread {
  struct task *task_info;
  cll_node rq_entry; /* Embedded list traversal struct for the runnable queue */
  cll_node task_node; /* Embedded list traversal struct for task */
  cll_node thrlist_entry; /* Threadlist node */
  thr_state_e state;
  mutex_s lock;
  int tid;
  thr_desched_e desched;
  void *sp;
  void *pc;
  swexn_t swexn;
  char *kstack;
};
typedef struct thread thread_t;

/* Thread manipulation */
thread_t *thread_init(struct task *t);
void thr_free(thread_t *t);
int thr_launch(thread_t *t, void *sp, void *pc);

/* Thread list manipulation */
int thrlist_add(thread_t *t);
int thrlist_del(thread_t *t);
thread_t *thrlist_find_and_lock(int tid);

#endif /* __THREAD_H__ */

