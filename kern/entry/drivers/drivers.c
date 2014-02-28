#include <idt.h>
#include <keyhelp.h>
#include <timer_defines.h>
#include "timer.h"

#include "driver_wrappers.h"

void install_device_handlers()
{
  install_interrupt_gate(KEY_IDT_ENTRY,asm_int_keyboard,IDT_KERN_DPL); 
  init_timer();
  install_interrupt_gate(TIMER_IDT_ENTRY,asm_int_timer,IDT_KERN_DPL); 
}
