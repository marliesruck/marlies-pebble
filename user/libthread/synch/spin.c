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

#include <syscall.h>
#include <atomic.h>


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
  while (sp->turn != turn) yield(-1);
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
  fetch_and_add(&(sp->turn), 1);
  return;
}

