#include <idt.h>
#include <keyhelp.h>
#include <timer_defines.h>
#include "timer.h"

#include "driver_wrappers.h"

void install_device_handlers()
{
  install_interrupt_gate(KEY_IDT_ENTRY, asm_kbd_int_handler,IDT_KERN_DPL);
  tmr_init(TMR_DEFAULT_RATE);
  install_interrupt_gate(TIMER_IDT_ENTRY, asm_tmr_int_handler, IDT_KERN_DPL); 
}
