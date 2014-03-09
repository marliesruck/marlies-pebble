/** @file fork_i.h
 *
 *  @brief Implements our system calls.
 *
 *  Even though any two system calls might do two VERY different things, we
 *  decided to implement them all in one file so that our user-facing API
 *  is in one place.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __FORK_I_H
#define __FORK_I_H

#include <process.h>
#include <thread.h>

#define CHILD_PDE (void *)(0xF0000000)

/* Retrieve parent kstack ESP and then compute child ESP as relative to its own
 * kstack */
#define KSTACK_LOW_OFFSET(esp,base)    ((unsigned int)(esp) - (unsigned int)(base))
#define KSTACK_HIGH_OFFSET(esp,high)    ((unsigned int)(high) - (unsigned int)(esp))


/* C protpotypes */
void *mem_map_child(void);
thread_t *init_child_tcb(void *child_cr3);

/* Asm protpotypes */
int finish_fork(void);

#endif /* __FORK_I_H */
