/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @author Fred Hacker (fhacker)
 *  @bug No known bugs.
 */

#include <common_kern.h>

/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */
#include <simics/simics.h>

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */
#include <x86/cr.h>

/* IDT specific includes */
#include <syscall_int.h>
#include <idt.h>
#include <keyhelp.h>
#include <timer_defines.h>
#include "drivers/driver_wrappers.h"
#include "drivers/timer.h"

/* Pebbles includes */
#include <vm.h>
#include <pg_table.h>
#include <frame_alloc.h>
#include <process.h>

/* Usr stack init includes */
#include <loader.h>
#include <usr_stack.h>


/*************************************************************************
 *  Random paging stuff that should really be elsewhere
 *************************************************************************/

#include <x86/cr.h>
#define CR0_PAGE 0x80000000


void enable_paging(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 |= CR0_PAGE;
  set_cr0(cr0);

  return;
}

void disable_paging(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 &= ~CR0_PAGE;
  set_cr0(cr0);

  return;
}


/*************************************************************************
 *  Kernel main
 *************************************************************************/

/** These does not belong here... */
void mode_switch(void *entry_point, void *sp);
int asm_sys_gettid(void);
//void asm_int_keyboard(void);
void install_fault_handlers(void);

/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
  lprintf( "Hello from a brand new kernel!" );

                      /* --- IDT setup --- */
  /* Exception handlers */
  install_trap_gate(GETTID_INT, asm_sys_gettid, IDT_USER_DPL);
  /* Hardware handlers...Should this be a trap? What if we get interrupted while
   * processing? */
  install_interrupt_gate(KEY_IDT_ENTRY,asm_int_keyboard,IDT_KERN_DPL); 
  init_timer();
  install_interrupt_gate(TIMER_IDT_ENTRY,asm_int_timer,IDT_KERN_DPL); 
  /* Fault handlers */
  install_fault_handlers(); 

  /* Set up kernel PTs and a PD */
  init_kern_pt();
  pde_t *pd = alloc_frame();
  init_pd(pd);
  set_cr3((uint32_t) pd);
  enable_paging();

  /* Load idle task and shamelessly hack our way into user-mode */
  void *entry_point = load_file("introspective");

  /* Initialize pg dir and tid in prototype tcb */
  my_pcb.pg_dir = (uint32_t)(pd);
  my_pcb.my_tcb.tid = 789;

  /* Set up usr stack */
  void *usr_sp = usr_stack_init();

  sim_reg_process(pd, "introspective");

#include <x86/eflags.h>

  lprintf("enabling interrupts...");
  enable_interrupts();
  //while(1);

  mode_switch(entry_point, usr_sp);

  return 0;
}

