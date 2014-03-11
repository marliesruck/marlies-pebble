/** @file fault_wrappers.h
 *
 *  @brief Declares assembly wrapper for the various faults.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#ifndef __FAULT_WRAPPERS_H__
#define __FAULT_WRAPPERS_H__


void asm_int_divzero(void);
void asm_int_debug(void);
void asm_int_nmi(void);
void asm_int_breakpoint(void);
void asm_int_overflow(void);
void asm_int_bound(void);
void asm_int_undef_opcode(void);
void asm_int_device_unavail(void);
void asm_int_double_fault(void);
void asm_int_cso(void);
void asm_int_tss(void);
void asm_int_seg_not_present(void);
void asm_int_stack_seg(void);
void asm_int_gen_prot(void);
void asm_int_float(void);
void asm_int_align(void);
void asm_int_machine_check(void);
void asm_int_simd(void);
void asm_int_generic(void);

/* @brief Page fault assembly routine.
 *
 * The page fault handler requires a custom wrapper because it's exception error
 * code is pushed on the the kernel handler stack by the processor.  It is
 * neccessary to retrieve this argument and pass it to the C function so it can
 * be given as an argument for a user defined page fault handler.
 *
 * @param error_code Error code pushed on by the processor
 *
 * @return Void.
 */
void asm_int_page_fault(void *error_code);

#endif /* __FAULT_WRAPPERS_H__ */

