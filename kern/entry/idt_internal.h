/** @file install_idt.h
 *
 * @brief Internal declarations for IDT manipulation
 *
 * @author Marlies Ruck(mruck)
 * @author Enrique Naudon (esn)
 *
 * @bug No known bugs
 **/
#ifndef __IDT_INTERNAL_H__
#define __IDT_INTERNAL_H__


/* Handler offset unpacking */
#define OFFSET_MSB(x)   ( ((unsigned int) x & 0xFFFF0000) >> 16)
#define OFFSET_LSB(x)   ( ((unsigned int) x & 0x0000FFFF) >> 0)

/* Gate type values */
#define TRAP_GATE       (0x0F) /* Trap gate bits */
#define INTERRUPT_GATE  (0x0E) /* Interrupt gate bits */

/** @struct idt_entry
 *
 *  @brief A page table entry.
 *
 *  We decided to use bitfields rather than doing the bit manipulation by
 *  hand so that the typechecker can check our work for us.
 *
 *  @var idt_entry::off_lo
 *  Bits [0,15] of the handler function's address.
 *
 *  @var idt_entry::segsel
 *  The segment selector.
 *
 *  @var idt_entry::resv
 *  Reserved bits; leave unset.
 *
 *  @var idt_entry::type
 *  The gate type of the entry (trap or interrupt).
 *
 *  @var idt_entry::dpl
 *  The desired priviledge level for this entry.
 *
 *  @var idt_entry::pres
 *  Present flag (1 = present).
 *
 *  @var idt_entry::off_hi
 *  Bits [16,32] of the handler function's address.
 **/
struct idt_entry {
  unsigned int off_lo : 16;
  unsigned int segsel : 16;
  unsigned int resv   : 8;
  unsigned int type   : 5;
  unsigned int dpl    : 2;
  unsigned int pres   : 1;
  unsigned int off_hi : 16;
};
typedef struct idt_entry idt_ent_s;


#endif /* __IDT_H__ */

