/** @file thr_internals.h
 *
 *  @brief This file may be used to define things internal to the thread
 *  library.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <tcb.h>


/* The root thread's TCB */
tcb_s main_tcb;

/* Size of thread stacks */
unsigned thread_stack_size;

/* The thread list */
thrlist_s thread_list;

/* Thread fork wrapper (child does not return!!) */
int thread_fork(void *stack);

/* Remove pages then vanish (w/o using memory) */
void remove_and_vanish(void *base, void **listp);


#endif /* THR_INTERNALS_H */
