/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *  
 *  A note on enabling interrupts: we do this relatively late in kernel main,
 *  and consequently accumulate some drift.  However, an invariant of our
 *  context switcher is that there is at least one running/runnable thread.  In
 *  order to guarantee this invariant, we need to initialize the first thread
 *  before enabling interrupts.
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @author Fred Hacker (fhacker)
 *  @bug No known bugs.
 */

#include <common_kern.h>

/* libc includes. */
#include <assert.h>
#include <malloc.h>
#include <simics.h>                 /* lprintf() */
#include <simics/simics.h>
#include <stdio.h>
#include <string.h>

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */
#include <x86/cr.h>

/* Pebbles includes */
#include <console.h>
#include <frame_alloc.h>
#include <idt.h>
#include <loader.h>
#include <pg_table.h>
#include <process.h>
#include <sched.h>
#include <sc_utils.h>
#include <thread.h>
#include <usr_stack.h>
#include <vm.h>

/* ZFOD includes */

/*************************************************************************
 *  Random paging stuff that should really be elsewhere
 *************************************************************************/

#include <x86/cr.h>


void enable_write_protect(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 |= CR0_WP;
  set_cr0(cr0);

  return;
}

void enable_paging(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 |= CR0_PG;
  set_cr0(cr0);

  return;
}

void disable_paging(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 &= ~CR0_PG;
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
  clear_console();

  vm_init_allocator();

  /* First executable page directory */
  pte_t *pd = retrieve_head();
  update_head(*(void **)pd);
  init_pd(pd, pd);

  /* Enable paging */
  set_cr3((uint32_t) pd);
  enable_write_protect();
  enable_paging();

  /* Load the first executable */
  thread_t *thread = load_task(pd, "init");

  /* Init curr and enable interrupts */
  enable_interrupts();

  /* Keep track of init's task */
  init = curr->task_info;

  /* Give up the kernel stack that was given to us by the bootloader */
  set_esp0((uint32_t)(&thread->kstack[KSTACK_SIZE]));

  /* Head to user-space */
  mode_switch(thread->pc, thread->sp);

  /* We should never reach here! */
  assert(0);
  return 0;
}

thread_t *load_task(void *pd, const char *fname)
{
  thread_t *thread = task_init();
  curr = thread;

  task_t *task = thread->task_info;

  /* Initialize pg dir and tid in prototype tcb */
  task->cr3 = (uint32_t)(pd);
  thread->task_info = task;

  /* Prepare to drop into user mode */
  thread->pc = load_file(&task->vmi, fname);
  thread->sp = usr_stack_init(&task->vmi, NULL);

  /* This enables interrupts */
  assert( sched_unblock(thread, 0) == 0 );

  sim_reg_process(pd, fname);
  return thread;
}

