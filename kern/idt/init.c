/** @file init.h
 *
 *  @brief Installs handlers in IDT
 *
 *  @author Marlies Ruck(mruck)
 *  @bug No known bugs
 */
#include <p1kern.h>
#include <stdlib.h>
#include <asm.h>
#include <seg.h>
#include <timer_defines.h>
#include <keyhelp.h>
#include "inc/init_i.h"
#include "inc/timer_i.h"
#include "inc/asm_wrappers.h"

/** @brief Generic handler installation function
 *
 *  @param index IDT index
 *  @param handler address of handler
 */
void load_idt_entry(int index, unsigned handler){
	char *base = idt_base();
	unsigned *word_one = (unsigned*)(base + (index * GATE_SIZE));
	unsigned *word_two = word_one + 1;
	*word_one =  (SEGSEL_KERNEL_CS << 16)| OFFSET_LSB(handler);
	*word_two = OFFSET_MSB(handler) | FLAGS;
	return;
}
