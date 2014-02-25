/** @file idt.h
 * *
 * * @brief Load entry into IDT
 * *
 * * @author Marlies Ruck(mruck)
 * * @bug No known bugs
 * */
#ifndef _IDT_H
#define _IDT_H

#define OFFSET_MSB(x) (0xFFFF0000 & (unsigned int) x) /* offset 31:16 */
#define OFFSET_LSB(x) (0x0000FFFF & (unsigned int) x) /* offset 15:0 */
#define TRAP_GATE (0x00000F00) /* Trap gate bits */
#define INTERRUPT_GATE (0x00000E00) /* Interrupt gate bits */
#define PRESENT_BIT (0x00008000) /* Present bit */
#define IDT_USER_DPL (0x00006000) /* This handler is user callable */
#define IDT_KERN_DPL (0x00000000) /* This handler must be called by
                                     the kernel */
/* Since a gate is 64 bits, pack into ULL */
#define PACK_IDT(hi,lo)\
  (((unsigned long long)(hi) << 32) | (unsigned long long)(lo))

void install_trap_gate(int index, void *handler, unsigned int dpl);
void install_interrupt_gate(int index, void *handler, unsigned int dpl);

#endif


