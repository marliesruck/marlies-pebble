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

/* Pebbles specific includes */
#include <atomic.h>
#include <sched.h>

/* Libc specific includes */
#include <assert.h>


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
 *  It is obviously possible to unlock a lock that you do not own; in the
 *  interest of keeping spinlocks as light as possible, we leave it to the
 *  caller to avoid doing this.
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

