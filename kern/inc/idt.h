/** @file idt.h
 *
 * @brief Declares our IDT manipulation API.
 *
 * @author Marlies Ruck(mruck)
 * @author Enrique Naudon (esn)
 *
 * @bug No known bugs
 **/
#ifndef __IDT_H__
#define __IDT_H__


/* IDT DPL flags */
#define IDT_USER_DPL (0x00006000) /* This handler is user callable */
#define IDT_KERN_DPL (0x00000000) /* This handler is kernel callable */

/* Setting IDT entries */
void install_trap_gate(int index, void *handler, unsigned int dpl);
void install_interrupt_gate(int index, void *handler, unsigned int dpl);

/* Installing interrupt handlers */
void install_device_handlers(void);
void install_fault_handlers(void);


#endif /* __IDT_H__ */

