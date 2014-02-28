/** @file fault_wrappers.S
 *
 *  @brief Implements assembly wrapper for the various faults.
 *
 *  See /kern/asm/intwrappers.h for the expanded INTERRUPT macro.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#include <asm/intwrappers.h>


VOID_INTERRUPT int_divzero
VOID_INTERRUPT int_debug
VOID_INTERRUPT int_nmi
VOID_INTERRUPT int_breakpoint
VOID_INTERRUPT int_overflow
VOID_INTERRUPT int_bound
VOID_INTERRUPT int_undef_opcode
VOID_INTERRUPT int_device_unavail
VOID_INTERRUPT int_double_fault
VOID_INTERRUPT int_cso
VOID_INTERRUPT int_tss
VOID_INTERRUPT int_seg_not_present
VOID_INTERRUPT int_stack_seg
VOID_INTERRUPT int_gen_prot
VOID_INTERRUPT int_page_fault
VOID_INTERRUPT int_float
VOID_INTERRUPT int_align
VOID_INTERRUPT int_machine_check
VOID_INTERRUPT int_simd
VOID_INTERRUPT int_generic

