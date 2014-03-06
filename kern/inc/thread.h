/* @file thread.h
 *
 */
#ifndef __THREAD_H__
#define __THREAD_H__

#include <process.h>
#include <vm.h>

#include <x86/page.h>
#include <stdint.h>

#define KSTACK_SIZE PAGE_SIZE

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

#include <process.h>
task_t task1;
task_t task2;

/* For fork debugging*/
thread_t child_pcb;

/* There should also be a curr tcb so we only flush the tlb when task switching */
thread_t *curr;

/*
void tcb_init(tcb_s *tcb, int tid, void *stack);
void tcb_final(tcb_s *tcb);
inline void tcb_lock(tcb_s *tcb);
inline void tcb_unlock(tcb_s *tcb);

void tcb_init(tcb_s *tcb, int tid, void *stack);
void tcb_final(tcb_s *tcb);
inline void tcb_lock(tcb_s *tcb);
inline void tcb_unlock(tcb_s *tcb);

void thrlist_init(tcb_s *tcb);
void thrlist_add(tcb_s *tcb);
void thrlist_del(tcb_s *tcb);
tcb_s *thrlist_findtcb(int tid);
tcb_s *thrlist_owntcb(void);
inline void thrlist_lock(void);
inline void thrlist_unlock(void);
*/




#endif /* _THREAD_H */
