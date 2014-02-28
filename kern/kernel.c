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
#include "entry/drivers/timer.h"
#include "entry/syscall/syscall_wrappers.h"

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

  /* Hardware interrupts */
  install_device_handlers();

  /* Fault handlers */
  install_fault_handlers(); 

  /* System calls */
  install_sys_handlers(); 


  /* Enable interrupts */
  enable_interrupts();

  /*  */
  init_kern_pt();

                  /* --- Hand load 2 executables for ctx switching  --- */

  /* First executable page directory */
  pte_s *pd = alloc_frame();
  init_pd(pd);

  /* Second executebale page directory */
  pte_s *pd2 = alloc_frame();
  init_pd(pd2);

                        /* --- Map first executable --- */ 

  set_cr3((uint32_t) pd);
  enable_paging();

  /* Initialize pg dir and tid in prototype tcb */
  my_pcb.vmi = (vm_info_s) {
    .pg_dir = (pte_s *)(TBL_HIGH),
    .pg_tbls = (pt_t *)(DIR_HIGH),
    .mmap = CLL_LIST_INITIALIZER(my_pcb.vmi.mmap)
  };
  void *entry_point = load_file(&my_pcb.vmi,"introspective");

  /* Set up usr stack */
  void *usr_sp = usr_stack_init(&my_pcb.vmi);

  /* set tcb.pc = mode switch
   * set tcb.sp = kstack[HIGH - 1]
   * push args to moded switch
void mode_switch(void *entry_point, void *sp);
  Set up kstack for user -> kernel mode switch 
  set_esp0((uint32_t)(&my_pcb.my_tcb.kstack[KSTACK_SIZE - 1]));

*/

  my_pcb.my_tcb.tid = 789;
  my_pcb.cr3 = (uint32_t)(pd); 
  my_pcb.my_tcb.sp = usr_sp;
  my_pcb.my_tcb.pc = entry_point;

  /* Make the user task debuggable */
  sim_reg_process(pd, "introspective");

                        /* --- Map second executable --- */ 

  set_cr3((uint32_t) pd2);

  /* Initialize pg dir and tid in prototype tcb */
  your_pcb.vmi = (vm_info_s) {
    .pg_dir = (pte_s *)(TBL_HIGH),
    .pg_tbls = (pt_t *)(DIR_HIGH),
    .mmap = CLL_LIST_INITIALIZER(your_pcb.vmi.mmap)
  };
  entry_point = load_file(&your_pcb.vmi, "introvert");

  /* Set up usr stack */
  usr_sp = usr_stack_init(&your_pcb.vmi);

  /* Initialize pg dir and tid in prototype tcb */
  your_pcb.cr3 = (uint32_t)(pd2);
  your_pcb.my_tcb.tid = 123;
  
  /* Initializing running tcb */
  curr_pcb = &your_pcb;

  /* Enable keyboard interrupts so we can ctx switch! */
  enable_interrupts();

  /* Drop into user mode */
  mode_switch(entry_point, usr_sp);

  (void)(entry_point);
  while(1);
  return 0;
}

