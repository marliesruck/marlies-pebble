/** @file init_i.h
 *
 *  @brief Load entry into IDT
 *
 *  @author Marlies Ruck(mruck)
 *  @bug No known bugs
 */
#ifndef _INIT_I_H
#define _INIT_I_H

#define GATE_SIZE	8
#define OFFSET_MSB(x) (0xFFFF0000 & x) /* offset 31:16 */
#define OFFSET_LSB(x) (0x0000FFFF & x) /* offset 15:0 */
#define FLAGS		       0x8F00 	       /* present bit, DPL, D */

void load_idt_entry(int index, unsigned handler);

#endif

