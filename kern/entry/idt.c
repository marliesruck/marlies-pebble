/** @file install_idt.c
 *
 *  @brief Implements IDT-manipulating functions.
 *
 *  @author Marlies Ruck(mruck)
 *  @bug No known bugs
 */

#include "idt_internal.h"
#include <idt.h>

/* x86 specific includes */
#include <x86/asm.h>
#include <x86/seg.h>

/** @brief Install trap gate 
 *
 *  @param index IDT index
 *  @param handler address of handler
 *  @param dpl Descriptor privledge level (should be either USER_DPL or
 *  KERN_DPL)
 *          
 */
void install_trap_gate(int index, void *handler, unsigned int dpl)
{
  idt_ent_s *idt = idt_base();

  idt[index].off_lo = OFFSET_LSB(handler);
  idt[index].off_hi = OFFSET_MSB(handler);
  idt[index].segsel = SEGSEL_KERNEL_CS;
  idt[index].type = TRAP_GATE;
  idt[index].dpl = dpl;
  idt[index].pres = 1;
  idt[index].resv = 0;

  return;
}

/** @brief Install interrupt gate 
 *
 *  @param index IDT index
 *  @param handler address of handler
 *  @param dpl Descriptor privledge level (should be either USER_DPL or
 *  KERN_DPL)
 *          
 */
void install_interrupt_gate(int index, void *handler, unsigned int dpl)
{
  idt_ent_s *idt = idt_base();

  idt[index].off_lo = OFFSET_LSB(handler);
  idt[index].off_hi = OFFSET_MSB(handler);
  idt[index].segsel = SEGSEL_KERNEL_CS;
  idt[index].type = INTERRUPT_GATE;
  idt[index].dpl = dpl;
  idt[index].pres = 1;
  idt[index].resv = 0;

  return;
}
