/** @file spin.h
 *
 *  @brief This file defines the interface for our spinlocks.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug No known bugs
 */

#ifndef __SPIN_H__
#define __SPIN_H__


/* @brief A light-weight spin-lock object. */
 
/** @struct spin_lock
 *  @brief A non-blocking lock.
 **/
struct spin_lock {
  unsigned int ticket;          /**< Imposeses an order on CS entries **/
  volatile unsigned int turn;   /**< Currently serving **/
  int owner;                    /**< TID of current owner >**/
};
typedef struct spin_lock spin_s;

/** @brief Statically initialize a spinlock.
 *
 *  @return A statically initialized spinlock.
 **/
#define SPIN_INITIALIZER() {  \
  .ticket = 0,                \
  .turn = 0,                  \
}

/* Spinlock operations */
void spin_init(spin_s *sp);
void spin_lock(spin_s *sp);
void spin_unlock(spin_s *sp);


#endif /* __SPIN_H__ */

