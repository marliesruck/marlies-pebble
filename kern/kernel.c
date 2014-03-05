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
#include <assert.h>

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

/** This does not belong here... */
void mode_switch(void *entry_point, void *sp);

void load_task(void *pd, pcb_t *my_pcb, const char *fname);

/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
                      /* --- IDT setup --- */

  /* Install interrupt handlers */
  install_device_handlers();
  install_fault_handlers(); 
  install_sys_handlers(); 

  /* Enable interrupts */
  enable_interrupts();

  /* Initialized kernel page tables */
  init_kern_pt();

  /* First executable page directory */
  pte_s *pd = alloc_frame();
  init_pd(pd);

  set_cr3((uint32_t) pd);
  enable_paging();

  /* Load the first executable */
  load_task(pd, &pcb1, "introvert");
  lprintf("parent stack = [%p, %p)", pcb1.my_tcb.kstack,
          &pcb1.my_tcb.kstack[KSTACK_SIZE]);
  lprintf("child stack = [%p, %p)", pcb2.my_tcb.kstack,
          &pcb2.my_tcb.kstack[KSTACK_SIZE]);

  /* Give up the kernel stack that was given to us by the bootloader */
  set_esp0((uint32_t)(&pcb1.my_tcb.kstack[KSTACK_SIZE - 1]));

  mode_switch(pcb1.my_tcb.pc, pcb1.my_tcb.sp);

  /* We should never reach here! */
  assert(0);

  return 0;
}

void load_task(void *pd, pcb_t *my_pcb, const char *fname)
{
  /* Initialize vm struct */
  my_pcb->vmi = (vm_info_s) {
    .pg_info = (pg_info_s) {
      .pg_dir = (pte_s *)(TBL_HIGH),
      .pg_tbls = (pt_t *)(DIR_HIGH),
    },
    .mmap = CLL_LIST_INITIALIZER(my_pcb->vmi.mmap)
  };

  /* Initialize pg dir and tid in prototype tcb */
  my_pcb->cr3 = (uint32_t)(pd);
  my_pcb->my_tcb.tid = 123;
  
  /* Initialize running tcb */
  curr_pcb = my_pcb;

  /* Drop into user mode */
  void *entry_point = load_file(&my_pcb->vmi, fname);
  void *usr_sp = usr_stack_init(&my_pcb->vmi, NULL);

  my_pcb->my_tcb.sp = usr_sp;
  my_pcb->my_tcb.pc = entry_point;
}

