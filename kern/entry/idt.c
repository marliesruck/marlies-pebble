/** @file install_idt.c
 *
 *  @brief Implements IDT-manipulation functions.
 *
 *  @author Marlies Ruck(mruck)
 *  @bug No known bugs
 */

#include <idt.h>

/* x86 specific includes */
#include <x86/asm.h>
#include <x86/seg.h>

/* IDT entry flags */
#define TRAP_GATE (0x00000F00)        /* Trap gate bits */
#define INTERRUPT_GATE (0x00000E00)   /* Interrupt gate bits */
#define PRESENT_BIT (0x00008000)      /* Present bit */

/* Offset manipulation */
#define OFFSET_MSB(x) (0xFFFF0000 & (unsigned int) x)   /* offset 31:16 */
#define OFFSET_LSB(x) (0x0000FFFF & (unsigned int) x)   /* offset 15:0 */

/* Packs two 32-bit words into a 64-bit word */
#define PACK_IDT(hi,lo)   \
  ( ((unsigned long long) (hi) << 32) | ((unsigned long long) (lo)) )

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
  unsigned long hi, lo;
	unsigned long long *idt = idt_base();

	lo = (SEGSEL_KERNEL_CS << 16) | OFFSET_LSB(handler);
	hi = OFFSET_MSB(handler) | (PRESENT_BIT | TRAP_GATE | dpl);

  idt[index] = PACK_IDT(hi,lo);
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
  unsigned long hi, lo;
	unsigned long long *idt = idt_base();

	lo = (SEGSEL_KERNEL_CS << 16) | OFFSET_LSB(handler);
	hi = OFFSET_MSB(handler) | (PRESENT_BIT | INTERRUPT_GATE | dpl);

  idt[index] = PACK_IDT(hi,lo);
	return;
}

