/** @file atomic.h
 *
 *  @brief Provides C declarations for atomic assembly instructions.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *  @bug No known bug
 */

#ifndef __ATOMIC_H__
#define __ATOMIC_H__

unsigned int fetch_and_add(volatile unsigned int *addr,
                           unsigned int addend);

unsigned int compare_and_swap(volatile unsigned int *addr,
                              unsigned int expect, unsigned int new);

#endif
