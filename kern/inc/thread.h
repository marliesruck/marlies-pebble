/* @file thread.h
 *
 */
#ifndef __THREAD_H__
#define __THREAD_H__

#include <process.h>
#include <queue.h>
#include <vm.h>

#include <x86/page.h>
#include <stdint.h>

#define KSTACK_SIZE PAGE_SIZE

queue_s naive_thrlist;

typedef struct thread{
  task_t *task_info;
  int tid;
  void *sp;
  void *pc;
  char kstack[KSTACK_SIZE];
}thread_t;

/* For exec debugging */
thread_t thread1;
thread_t thread2;

task_t task1;
task_t task2;

/* For fork debugging*/
thread_t child_pcb;

/* Keep track of running thread */
thread_t *curr;

/* Initialization routines */
thread_t *task_init(void);
thread_t *thread_init(task_t *task);
void thrlist_init(queue_s *q);

/* Manipulation routines */
void thrlist_enqueue(thread_t *thread, queue_s *q);
thread_t *thrlist_dequeue(queue_s *q);

#endif /* _THREAD_H */
