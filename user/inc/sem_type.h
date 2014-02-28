/** @file sem_type.h
 *
 *  @brief This file defines the type for semaphores.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug No known bugs
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include <spin.h>
#include <queue.h>

typedef struct sem {
  spin_s lock; 
  int count;
  queue_s queue;
} sem_t;

#endif /* _SEM_TYPE_H */
