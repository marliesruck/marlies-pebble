/* @file process.h
 *
 */
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <vm.h>

#include <x86/page.h>
#include <stdint.h>
#include <cllist.h>
#include <mutex.h>
#include  <cvar.h>

extern mutex_s task_list_lock;

typedef struct task{
  struct task *parent;    /* Enqueue the last thread_t to exit in my parent's
                             list of dead children...that makes the parent
                             responsible for reaping that thread_t */
  queue_s dead_children;  /* List of threads for MY dead children to enqueue 
                             themselves in.  This is the list I wait() on */
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
  mutex_s  lock;           /* Hold this lock when modifying the task struct */
  int status;
}task_t;

/* This does not belong here */
task_t *init;

/* Task list manipulation routines */
int tasklist_add(task_t *t);
int tasklist_del(task_t *t);
task_t *tasklist_find(task_t *t);


#endif /* _PROCESS_H */
