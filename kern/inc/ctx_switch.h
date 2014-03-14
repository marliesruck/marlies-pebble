/** @file ctx_switch.h
 *
 *  @brief Declares our ctx_switch API.
 *
 *  TODO: We might consider declaring a wrapper around ctx_switch that
 *  pulls the requisite fields from the thread struct...
 *
 *  Note: I did the comments in the .h file since this API contains asm
 *  functions
 *
 *  @author Marlies Ruck(mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 **/
#ifndef __CTX_SWITCH_H__
#define __CTX_SWITCH_H__

#include <thread.h>


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
 *  @param prev_sp      Address for storing previous thread's ESP.  @param
 *  prev_pc      Address for storing previous thread's EIP.  @param next_sp
 *  Next thread's ESP.  @param next_pc      Next thread's EIP.  @param
 *  next_cr3     Next thread's cr3.  @param kstack_high  Value for next
 *  thread's esp0.
 *
 *  @return Void.
 */
void ctx_switch(void *prev_sp, void *prev_pc, void *next_sp, void *next_pc,
                unsigned int next_cr3, void *kstack_high);


#endif /* __CTX_SWITCH_H__ */

