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
#include "syscall/syscall_wrappers.h"

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
  install_trap_gate(EXEC_INT, asm_sys_exec, IDT_USER_DPL);
  install_trap_gate(GETCHAR_INT, asm_sys_getchar, IDT_USER_DPL);

  /* Hardware handlers  */
  install_interrupt_gate(KEY_IDT_ENTRY,asm_int_keyboard,IDT_KERN_DPL); 
  init_timer();
  install_interrupt_gate(TIMER_IDT_ENTRY,asm_int_timer,IDT_KERN_DPL); 

  /* Fault handlers */
  install_fault_handlers(); 

                  /* --- Hand load 2 executables for ctx switching  --- */
  init_kern_pt();

  /* First executable page directory */
  pde_t *pd = alloc_frame();
  init_pd(pd);

  /* Second executebale page directory */
  pde_t *pd2 = alloc_frame();
  init_pd(pd2);

                        /* --- Map first executable --- */ 

  set_cr3((uint32_t) pd);
  enable_paging();

  /* Initialize pg dir and tid in prototype tcb */
  my_pcb.vmi = (vm_info_s) {
    .pg_dir = (pde_t *)(TBL_HIGH),
    .pg_tbls = (pt_t *)(DIR_HIGH),
    .mmap = CLL_LIST_INITIALIZER(my_pcb.vmi.mmap)
  };
  void *entry_point = load_file(&my_pcb.vmi,"introspective");

  /* Set up usr stack */
  void *usr_sp = usr_stack_init(&my_pcb.vmi);

  my_pcb.my_tcb.tid = 789;
  my_pcb.pg_dir = (uint32_t)(pd); /* REDUNDANT */
  my_pcb.my_tcb.sp = usr_sp;
  my_pcb.my_tcb.pc = entry_point;

  /* Make the user task debuggable */
  sim_reg_process(pd, "introspective");

                        /* --- Map second executable --- */ 

  set_cr3((uint32_t) pd2);

  /* Initialize pg dir and tid in prototype tcb */
  your_pcb.vmi = (vm_info_s) {
    .pg_dir = (pde_t *)(TBL_HIGH),
    .pg_tbls = (pt_t *)(DIR_HIGH),
    .mmap = CLL_LIST_INITIALIZER(your_pcb.vmi.mmap)
  };
  entry_point = load_file(&your_pcb.vmi, "introvert");

  /* Set up usr stack */
  usr_sp = usr_stack_init(&your_pcb.vmi);

  /* Initialize pg dir and tid in prototype tcb */
  your_pcb.pg_dir = (uint32_t)(pd2);
  your_pcb.my_tcb.tid = 123;
  
  /* Enable keyboard interrupts so we can ctx switch! */
  enable_interrupts();

  /* Drop into user mode */
  mode_switch(entry_point, usr_sp);

  (void)(entry_point);
  while(1);
  return 0;
}

