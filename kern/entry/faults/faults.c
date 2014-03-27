/** @file faults.c
 *
 *  @brief Implements our various fault handlers.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <assert.h>
#include <simics.h>
#include <x86/idt.h>

#include <idt.h>
#include "fault_wrappers.h"

/** @brief Installs our fault handlers.
 *
 *  @return Void.
 **/
void install_fault_handlers(void)
{
  install_trap_gate(IDT_DE, asm_int_divzero, IDT_KERN_DPL);
  install_trap_gate(IDT_DB, asm_int_debug, IDT_KERN_DPL);
  install_trap_gate(IDT_NMI, asm_int_nmi, IDT_KERN_DPL);
  install_trap_gate(IDT_BP, asm_int_breakpoint, IDT_KERN_DPL);
  install_trap_gate(IDT_OF, asm_int_overflow, IDT_KERN_DPL);
  install_trap_gate(IDT_BR, asm_int_bound, IDT_KERN_DPL);
  install_trap_gate(IDT_UD, asm_int_undef_opcode, IDT_KERN_DPL);
  install_trap_gate(IDT_NM, asm_int_device_unavail, IDT_KERN_DPL);
  install_trap_gate(IDT_DF, asm_int_double_fault, IDT_KERN_DPL);
  install_trap_gate(IDT_CSO, asm_int_cso, IDT_KERN_DPL);
  install_trap_gate(IDT_TS, asm_int_tss, IDT_KERN_DPL);
  install_trap_gate(IDT_NP, asm_int_seg_not_present, IDT_KERN_DPL);
  install_trap_gate(IDT_SS, asm_int_stack_seg, IDT_KERN_DPL);
  install_trap_gate(IDT_GP, asm_int_gen_prot, IDT_KERN_DPL);
  install_trap_gate(IDT_PF, asm_int_page_fault, IDT_KERN_DPL);
  install_trap_gate(IDT_MF, asm_int_float, IDT_KERN_DPL);
  install_trap_gate(IDT_AC, asm_int_align, IDT_KERN_DPL);
  install_trap_gate(IDT_MC, asm_int_machine_check, IDT_KERN_DPL);
  install_trap_gate(IDT_XF, asm_int_simd, IDT_KERN_DPL);

  return;
}

/** @brief Handles division by zero.
 *
 *  @return Void.
 **/
void int_divzero(void)
{
  lprintf("Error: Division by zero!");
  panic("Error: Division by zero!!");
  return;
}

/** @brief Handles debug interrupts.
 *
 *  @return Void.
 **/
void int_debug(void)
{
  lprintf("Alert: Got debug interrupt...");
  panic("Alert: Got debug interrupt...");
  return;
}

/** @brief Handles non-maskable interrupts.
 *
 *  @return Void.
 **/
void int_nmi(void)
{
  lprintf("Error: Non-maskable interrupt!");
  panic("Error: Non-maskable interrupt!");
  return;
}

/** @brief Handles breakpoint interrupts.
 *
 *  @return Void.
 **/
void int_breakpoint(void)
{
  lprintf("Alert: Encountered breakpoint (INT 3)!");
  panic("Alert: Encountered breakpoint (INT 3)!");
  return;
}

/** @brief Handles overflow exceptions.
 *
 *  @return Void.
 **/
void int_overflow(void)
{
  lprintf("Error: Overflow (INTO)!");
  panic("Error: Overflow (INTO)!");
  return;
}

/** @brief Handles out of bounds exceptions.
 *
 *  @return Void.
 **/
void int_bound(void)
{
  lprintf("Error: Range exceeded (BOUND)!");
  panic("Error: Range exceeded (BOUND)!");
  return;
}

/** @brief Handles invalid instructions.
 *
 *  @return Void.
 **/
void int_undef_opcode(void)
{
  lprintf("Error: Invalid instruction!");
  panic("Error: Invalid instruction!");
  return;
}

/** @brief Handles unavailable devices.
 *
 *  @return Void.
 **/
void int_device_unavail(void)
{
  lprintf("Error: Device not available!");
  panic("Error: Device not available!");
  return;
}

/** @brief Handles double faults.
 *
 *  @return Void.
 **/
void int_double_fault(void)
{
  lprintf("Error: Double fault!");
  panic("Error: Double fault!");
  return;
}

/** @brief Handles coprocessor segment overruns.
 *
 *  @return Void.
 **/
void int_cso(void)
{
  lprintf("Error: Coprocessor segment overrun!");
  panic("Error: Coprocessor segment overrun!");
  return;
}

/** @brief Handles invalid task segment selectors.
 *
 *  @return Void.
 **/
void int_tss(void)
{
  lprintf("Error: Invalid task segment selector!");
  panic("Error: Invalid task segment selector!");
  return;
}

/** @brief Handles non-present segments.
 *
 *  @return Void.
 **/
void int_seg_not_present(void)
{
  lprintf("Error: Segment not present!");
  panic("Error: Segment not present!");
  return;
}

/** @brief Handles stack segment faults.
 *
 *  @return Void.
 **/
void int_stack_seg(void)
{
  lprintf("Error: Stack segment fault!");
  panic("Error: Stack segment fault!");
  return;
}

/** @brief Handles general protection faults.
 *
 *  @return Void.
 **/
void int_gen_prot(void)
{
  lprintf("Error: General protection fault!");
  panic("Error: General protection fault!");
  return;
}

/** @brief Handles page faults.
 *
 *  @return Void.
 **/
/* Pebbles specific includes */
#include <sched.h>
/* x86 specific includes */
#include <x86/cr.h>
void int_page_fault(void *pc, void *error_code)
{
  void *addr;

  /* Grab the faulting address and page info */
  addr = (void *)get_cr2();

  /* Try to handle the fault */
  if (pg_page_fault_handler(addr))
  {
    lprintf("Error: Page fault on table-less address %p by instruction %p", 
            addr, pc);
    panic("Error: Page fault!");
  }

  return;
}

/** @brief Handles floating point exceptions.
 *
 *  @return Void.
 **/
void int_float(void)
{
  lprintf("Error: Floating point exception!");
  panic("Error: Floating point exception!");
  return;
}

/** @brief Handles (failed?) alignment checks.
 *
 *  @return Void.
 **/
void int_align(void)
{
  lprintf("Error: Alignment check!");
  panic("Error: Alignment check!");
  return;
}

/** @brief Handles machine checks.
 *
 *  @return Void.
 **/
void int_machine_check(void)
{
  lprintf("Error: Machine check!");
  panic("Error: Machine check!");
  return;
}

/** @brief Handles SIMD exceptions.
 *
 *  @return Void.
 **/
void int_simd(void)
{
  lprintf("Error: SIMD floating point exception!");
  panic("Error: SIMD floating point exception!");
  return;
}

/** @brief Handles pretty much anything else.
 *
 *  @return Void.
 **/
void int_generic(void)
{
  lprintf("Error: Got a fault!");
  panic("Error: Got a fault!");
  return;
}

