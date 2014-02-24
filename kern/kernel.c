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

/* Pebbles includes */
#include <vm.h>
#include <pg_table.h>
#include <frame_alloc.h>
#include <loader.h>
#include <syscall_int.h>
#include "idt/inc/init_i.h"


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

/** These do not belong here... */
void mode_switch(uint32_t eip, uint32_t esp);
int asm_sys_gettid(void);
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

  /* IDT setup */
  install_trap_gate(GETTID_INT, asm_sys_gettid, IDT_USER_DPL);
  install_fault_handlers();

  /* Set up kernel PTs and a PD */
  init_kern_pt();
  pde_t *pd = alloc_frame();
  init_pd(pd);
  set_cr3((uint32_t) pd);
  enable_paging();

  /* Load idle task and shamelessly hack our way into user-mode */
  load_file("introspective");
  vm_alloc(pg_dir, (void *)DIR_HIGH-1, 1,
           PG_TBL_PRESENT|PG_TBL_WRITABLE|PG_TBL_USER);
  sim_reg_process(pd, "introspective");
  uint32_t stack;
  set_esp0((uint32_t) &stack);
  stack = 0;
  mode_switch(USER_MEM_START, DIR_HIGH);

  return 0;
}

