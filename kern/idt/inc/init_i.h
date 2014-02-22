/** @file init_i.h
 *
 *  @brief Load entry into IDT
 *
 *  @author Marlies Ruck(mruck)
 *  @bug No known bugs
 */
#ifndef _INIT_I_H
#define _INIT_I_H

#define OFFSET_MSB(x)  (0xFFFF0000 & x) /* offset 31:16 */
#define OFFSET_LSB(x)  (0x0000FFFF & x) /* offset 15:0 */
#define TRAP_GATE      (0x00000F00)     /* Trap gate bits */
#define INTERRUPT_GATE (0x00000E00)     /* Interrupt gate bits */
#define PRESENT_BIT    (0x00008000)     /* Present bit */
#define USER_DPL       (0x00000300)     /* This handler is user callable */
#define KERN_DPL       (0x00000000)     /* This handler must be called by 
                                           the kernel */
/* Since a gate is 64 bits, pack into ULL */
#define PACK_IDT(hi,lo)\
    (((unsigned long long)(hi) << 32) | (unsigned long long)(lo)) 

void install_trap_gate(int index, unsigned handler, unsigned int dpl);
void install_interrupt_gate(int index, unsigned handler, unsigned int dpl);

#endif

