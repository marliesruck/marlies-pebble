/** @file spin.c
 *
 *  @brief This file implements our spinlocks.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 */

#include <spin.h>

/* Pebbles includes */
#include <atomic.h>
#include <sched.h>

/* Libc includes */
#include <assert.h>

/* x86 includes */
#include <x86/asm.h>


/** @brief Initialize a spinlock.
 *
 *  The spinlock is initialy set to the unlocked state.
 *
 *  @param sp The spinlock to initialize.
 *
 *  @return Void.
 **/
inline void spin_init(spin_s *sp)
{
  sp->ticket = 0;
  sp->turn = 0;
  sp->owner = -1;

  return;
}

/** @brief Lock a spinlock.
 *
 *  @param sp The spinlock to lock.
 *
 *  @return Void.
 **/
inline void spin_lock(spin_s *sp)
{
  int turn;
  turn = fetch_and_add(&(sp->ticket), 1);
  while (sp->turn != turn) continue;
  sp->owner = curr_thr->tid;
  return;
}

/** @brief Unlock a spinlock.
 *
 *  @param sp The spinlock to unlock.
 *
 *  @return Void.
 **/
inline void spin_unlock(spin_s *sp)
{
  assert(sp->owner == curr_thr->tid);
  sp->owner = -1;
  fetch_and_add(&(sp->turn), 1);
  return;
}

/** @brief Atomically unlock a spinlock and block the caller.
 *
 *  @param sp The spinlock to unlock.
 *
 *  @return Void.
 **/
inline void spin_unlock_and_block(spin_s *sp)
{
  disable_interrupts();

  /* Unlock the spinlock */
  spin_unlock(sp);

  /* Deschedule yourself */
  rq_del(curr_thr);
  schedule_unprotected();

  enable_interrupts();
  return;
}

