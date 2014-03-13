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


VOID_INTERRUPT tmr_int_handler
VOID_INTERRUPT kbd_int_handler

