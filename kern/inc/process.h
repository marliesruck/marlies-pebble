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

extern mutex_s task_list_lock;

typedef struct task{
  struct task *parent;    /* Responsible for reaping my dead children */
  cll_list dead_children; /* The list I wait() on */
  cvar_s  cv;             /* For the parent to sleep on while it's waiting to
                             reap its children */
  cll_list peer_threads;  /* As the task spawns thread, add them here. When the
                             last thread vanishes, we traverse this list and
                             free all threads but ourselves */
  uint32_t cr3;           /* PTBR */
  vm_info_s vmi;          /* Virtual Memory */
  uint32_t num_threads;   /* For knowing when vanish() should 
                             deallocate ALL resources */
  int orig_tid;           /* Wait() returns the TID of the origin thread of the 
                             exiting tasks, not the tid of the last thread 
                             to vanish */
  int live_children;      /* This and dead_children determine whether or not
                             wait should block */
  mutex_s  lock;          /* Hold this lock when modifying the task struct */
  int status;             /* My exit status */
}task_t;

/* Keep track of init for zombie children */
task_t *init;

/* Task manipulation routines */
struct thread *task_init(void);

/* Task list manipulation routines */
int tasklist_add(task_t *t);
int tasklist_del(task_t *t);
task_t *tasklist_find(task_t *t);

#endif /* _PROCESS_H */
