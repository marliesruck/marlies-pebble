/** @file tcb.h
 *
 *  @brief Declares the tcb struct and associated functions.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 **/
#ifndef __TCB_H__
#define __TCB_H__

#include <cllist.h>
#include <mutex.h>

/** @brief Flag values for thread data state.
 **/
enum tcb_state {
  THR_NASCENT,
  THR_ACTIVE,
  THR_EXITED,
};
typedef enum tcb_state tcb_state_e;

struct tcb {
  mutex_t lock;
  tcb_state_e state;
	int tid;
	int reject; 
  void *stack;
	void *status;
	struct tcb *joinp;
};
typedef struct tcb tcb_s;

struct thread_list {
  cll_list list;
  mutex_t lock;
};
typedef struct thread_list thrlist_s;

/* TCB functions */
void tcb_init(tcb_s *tcb, int tid, void *stack);
void tcb_final(tcb_s *tcb);
inline void tcb_lock(tcb_s *tcb);
inline void tcb_unlock(tcb_s *tcb);

/* Thread list functions */
void thrlist_init(tcb_s *tcb);
void thrlist_add(tcb_s *tcb);
void thrlist_del(tcb_s *tcb);
tcb_s *thrlist_findtcb(int tid);
tcb_s *thrlist_owntcb(void);
inline void thrlist_lock(void);
inline void thrlist_unlock(void);


#endif /* __TCB_H__ */

