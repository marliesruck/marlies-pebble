/** @file init.h
 *
 *  @brief Installs handlers in IDT
 *
 *  @author Marlies Ruck(mruck)
 *  @bug No known bugs
 */
#include "inc/init_i.h"

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
void install_trap_gate(int index, unsigned handler,unsigned int dpl)
{
	unsigned long long *idt = idt_base();
	unsigned int word_one =  (SEGSEL_KERNEL_CS << 16)| OFFSET_LSB(handler);
	unsigned int word_two = OFFSET_MSB(handler) | (PRESENT_BIT | TRAP_GATE | dpl);
  idt[index] = PACK_IDT(word_two,word_one);
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
void install_interrupt_gate(int index, unsigned handler,unsigned int dpl)
{
	unsigned long long *idt = idt_base();
	unsigned int word_one =  (SEGSEL_KERNEL_CS << 16)| OFFSET_LSB(handler);
	unsigned int word_two = OFFSET_MSB(handler) | (PRESENT_BIT | INTERRUPT_GATE | dpl);
  idt[index] = PACK_IDT(word_two,word_one);
	return;
}
