/** @file fault_wrappers.S
 *
 *  @brief Implements assembly wrapper for the various faults.
 *
 *  See /kern/asm/intwrappers.h for the expanded FAULT_* macros.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#include <asm/intwrappers.h>

/*** --- Pebbles interface does not allow users 
         to install handlers for these faults --- ***/

VOID_FAULT int_nmi
VOID_FAULT_ERROR int_double_fault
VOID_FAULT int_cso
VOID_FAULT_ERROR int_tss
VOID_FAULT int_machine_check
VOID_FAULT int_generic

/*** --- Faults that users can install a heandler for --- ***/

VOID_FAULT_SWEXN int_divzero
VOID_FAULT_SWEXN int_debug
VOID_FAULT_SWEXN int_breakpoint
VOID_FAULT_SWEXN int_overflow
VOID_FAULT_SWEXN int_bound
VOID_FAULT_SWEXN int_undef_opcode
VOID_FAULT_SWEXN int_device_unavail
VOID_FAULT_ERROR_SWEXN int_seg_not_present
VOID_FAULT_ERROR_SWEXN int_stack_seg
VOID_FAULT_ERROR_SWEXN int_gen_prot
VOID_FAULT_ERROR_SWEXN int_page_fault
VOID_FAULT_SWEXN int_float
VOID_FAULT_ERROR_SWEXN int_align
VOID_FAULT_SWEXN int_simd




