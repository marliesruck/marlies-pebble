/** @file dispatch.h
 *
 *  @brief Assembly and C functions for mode switches.
 *
 *  Dispatch() is an assembly function for context switching and half_dispatch()
 *  is an assembly function specifically for transitioning from kernel to user
 *  mode.
 *
 *  @author Marlies Ruck(mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 **/

#ifndef __DISPATCH_H__
#define __DISPATCH_H__

#include <thread.h>

/** @brief Mode switch from kernel to user.
 *
 *  @param entry_point Value EIP should be set to.
 *  @param sp Vale ESP should be set to.
 *
 *  @return Void.
 **/
void half_dispatch(void *entry_point, void *sp);

/** @brief Wrapper for dispatch
 *
 *  @param next Thread to dispatch.
 *
 *  @return Void.
 **/
void dispatch_wrapper(thread_t *next);

/* @brief Switch between threads.
 * 
 * Stores all CPU state on the currently executing thread's stack except
 * for the program counter and stack pointer, which is done in
 * store_and_switch and stored in the thread_t data structure. When a
 * thread that is ctx_switched out runs, it returns to the instruction
 * immediately after the call to store_and_switch.  It's stack pointer and
 * pc are restored by the thread getting context switched out, while the
 * rest of the CPU state is on its kernel stack and it restores upon
 * returning from store_and_switch().
 *
 *  @param prev_sp      Address for storing previous thread's ESP.  
 *  @param prev_pc      Address for storing previous thread's EIP.  
 *  @param next_sp      Next thread's ESP. 
 *  @param next_pc      Next thread's EIP.  
 *  @param cr3          Next thread's cr3.  
 *  @param kstack_high  Value for next thread's esp0.
 *
 *  @return Void.
 */
void dispatch(void *prev_sp, void *prev_pc, void *next_sp, void *next_pc,
                unsigned int next_cr3, void *kstack_high);

#endif /* __DISPATCH_H__ */

