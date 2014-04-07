/** @file fault_wrappers.h
 *
 *  @brief Declares assembly wrapper for the various faults.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#ifndef __FAULT_WRAPPERS_H__
#define __FAULT_WRAPPERS_H__

#include <ureg.h>

int asm_int_divzero(ureg_t *ureg);
int asm_int_debug(ureg_t *ureg);
void asm_int_nmi(void);
int asm_int_breakpoint(ureg_t *ureg);
int asm_int_overflow(ureg_t *ureg);
int asm_int_bound(ureg_t *ureg);
int asm_int_undef_opcode(ureg_t *ureg);
int asm_int_device_unavail(ureg_t *ureg);
void asm_int_double_fault(void);
void asm_int_cso(void);
void asm_int_tss(void);
int asm_int_seg_not_present(ureg_t *ureg);
int asm_int_stack_seg(ureg_t *ureg);
int asm_int_gen_prot(ureg_t *ureg);
int asm_int_float(ureg_t *ureg);
int asm_int_align(ureg_t *ureg);
void asm_int_machine_check(void);
int asm_int_simd(ureg_t *ureg);
void asm_int_generic(void);
int asm_int_page_fault(ureg_t *ureg);

#endif /* __FAULT_WRAPPERS_H__ */

