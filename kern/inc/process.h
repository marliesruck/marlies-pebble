/** @file process.h
 *
 *  @brief Declares structures and functions for task manipulation.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __PROCESS_H__
#define __PROCESS_H__

/* x86 specific inclues */
#include <x86/page.h>

/* Pebbles specific includes */
#include <vm.h>
#include <thread.h>
#include <cllist.h>
#include <mutex.h>
#include <cvar.h>

/* Libc specific includes */
#include <stdint.h>

struct mini_pcb {
  int tid;
  int status;
  cll_node entry;
};
typedef struct mini_pcb mini_pcb_s;

struct task {
  mini_pcb_s *mini_pcb;
  int parent_tid;         /* Tid of root thread in my parent */
  queue_s  dead_children; /* The list I wait() on */
  cvar_s  cv;             /* For the parent to sleep on while it's waiting to
                             reap its children */
  struct task *dead_task;
  struct thread *dead_thr;
  uint32_t cr3;           /* PTBR */
  vm_info_s vmi;          /* Virtual Memory */
  uint32_t num_threads;   /* For knowing when vanish() should 
                             deallocate ALL resources */
  int live_children;      /* This and dead_children determine whether or not
                             wait should block */
  mutex_s  lock;          /* Hold this lock when modifying the task struct */
  char *execname;          /* For simics and debugging in general...currently
                              a memory leak, get rid of this before submitting */
};
typedef struct task task_t;

/* Keep track of init for zombie children */
task_t *init;

/* Task manipulation routines */
struct thread *task_init(void);
void task_free(task_t *task);
void task_add_thread(task_t *tsk, struct thread *thr);
void task_del_thread(task_t *tsk, struct thread *thr);
task_t *task_signal_parent(task_t *task);
int task_find_zombie(task_t *task, int *status);
void task_reap(task_t *victim);

/* Task list manipulation routines */
void tasklist_add(task_t *t);
void tasklist_del(task_t *t);


#endif /* __PROCESS_H__ */

