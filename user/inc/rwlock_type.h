/** @file rwlock_type.h
 *
 *  @brief This file defines the type for reader/writer locks.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrqiue Naudon (esn)
 *  @bug No known bugs
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <spin.h>
#include <cllist.h>
#include <rwlock.h>


/** @brief Flag values for lock state.
 **/
enum rwlock_state {
  RWLOCK_UNLOCKED,
  RWLOCK_WRLOCKED,
  RWLOCK_RDLOCKED,
};
typedef enum rwlock_state rwlock_state_e;

typedef struct rwlock {
  spin_s lock;
  cll_list writers;
  cll_list readers_waiting;
  cll_list readers_reading;
  rwlock_state_e state;
  int writer_tid; /* Writer holding lock. Make sure they are the one
                    who relinquishes it */
} rwlock_t;


#endif /* _RWLOCK_TYPE_H */
