/* @file thread.h
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug No known bugs
 */
#ifndef __THREAD_H__
#define __THREAD_H__

#include <process.h>
#include <queue.h>
#include <vm.h>

#include <x86/page.h>
#include <stdint.h>

#define KSTACK_SIZE PAGE_SIZE

extern queue_s naive_thrlist;

typedef struct thread{
  task_t *task_info;
  int tid;
  void *sp;
  void *pc;
  char kstack[KSTACK_SIZE];
}thread_t;

/* Initialization routines */
thread_t *task_init(void);
thread_t *thread_init(task_t *task);

/* Manipulation routines */
void thrlist_enqueue(thread_t *thread, queue_s *q);
thread_t *thrlist_dequeue(queue_s *q);

#endif /* __THREAD_H__ */
