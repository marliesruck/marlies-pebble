/** @file driver_wrappers.S
 *
 *  @brief Implements assembly wrapper for the various drivers.
 *
 *  See /kern/asm/intwrappers.h for the expanded INTERRUPT macro.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#include <asm/intwrappers.h>

VOID_INTERRUPT int_keyboard 
VOID_INTERRUPT int_timer
