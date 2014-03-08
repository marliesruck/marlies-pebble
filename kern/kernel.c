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
#include <thread.h>
#include <ctx_switch.h>

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

thread_t *load_task(void *pd, const char *fname);

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

  /* Initialize naive thread list */
  thrlist_init(&naive_thrlist);

  /* Load the first executable */
  thread_t *thread1 = load_task(pd, "introvert");


  /* Give up the kernel stack that was given to us by the bootloader */
  set_esp0((uint32_t)(&thread1->kstack[KSTACK_SIZE - 1]));

  mode_switch(thread1->pc, thread1->sp);

  /* We should never reach here! */
  assert(0);

  return 0;
}

thread_t *load_task(void *pd, const char *fname)
{
  thread_t *thread1 = task_init();

  task_t *task = thread1->task_info;

  /* Initialize pg dir and tid in prototype tcb */
  task->cr3 = (uint32_t)(pd);
  thread1->task_info = task;
  
  /* Initialize currently running thread */
  curr = thread1;

  /* Prepare to drop into user mode */
  thread1->pc = load_file(&task->vmi, fname);
  thread1->sp = usr_stack_init(&task->vmi, NULL);

  return thread1;
}

