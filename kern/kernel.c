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
 **/

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
#include <page_util.h>

/* Pebbles includes */
#include <console.h>
#include <idt.h>
#include <frame_alloc.h>
#include <loader.h>
#include <process.h>
#include <sched.h>
#include <sc_utils.h>
#include <thread.h>
#include <usr_stack.h>
#include <vm.h>
#include <sched.h>
#include <dispatch.h>
#include <util.h>

/* --- Internal helper routines --- */
void init_kdata_structures(void);
void *raw_init_pd(void);
void init_stack(thread_t *thr);
thread_t *hand_load_task(const char *fname);

/*************************************************************************
 *  Kernel main
 *************************************************************************/

/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
  /* Initialize kernel data structures */
  init_kdata_structures();
  enable_write_protect();

  /* Hand load idle */
  thread_t *idle = hand_load_task("idle");
  init_stack(idle);

  /* Hand load init */
  hand_load_task("regression");

  /* Keep track of init's task */
  init = curr->task_info;

  /* Give up the kernel stack that was given to us by the bootloader */
  set_esp0((uint32_t)(&curr->kstack[KSTACK_SIZE]));

  /* Launch init and enter user space...The iret enables interrupts */
  half_dispatch(curr->pc, curr->sp);

  /* We should never reach here! */
  assert(0);

  /* To placate the compiler */
  return 0;
}

/** @brief Initialize kernel data structures.
 *
 *  Initializes IDT, frame allocator, kernel page tables and dummy frame for
 *  ZFOD.
 *
 *  @return Void.
 */
void init_kdata_structures(void)
{
  /* Install interrupt handlers */
  install_device_handlers();
  install_fault_handlers(); 
  install_sys_handlers(); 
  clear_console();

  /* Initalize memory modules...starts with virtual memory and peels away each
   * layer of abstraction until reaching the frame allocator */
  vm_init_allocator();

  return;
}

/** @brief Hand load a task.
 *
 *  @param pd Page directory in task struct.
 *  @param fname Filename of task.
 *
 *  @return Address of root thread.
 **/
thread_t *hand_load_task(const char *fname)
{
  thread_t *thread = task_init();
  curr = thread;
  task_t *task = thread->task_info;

  /* Enable paging */
  set_cr3(task->cr3);
  enable_paging();

  /* Prepare to drop into user mode */
  thread->pc = load_file(&task->vmi, fname);
  thread->sp = usr_stack_init(&task->vmi, NULL);

  /* Add the task to the runnable queue */
  raw_unblock(thread, &thread->node);

  sim_reg_process((void *)task->cr3, fname);
  return thread;
}


/** @brief Prepare the kernel stack for half_dispatch().
 *
 *  This is only used for making idle a valid scheduleable unit, but it can be
 *  called for any other thread that needs to initialize its kernel stack in
 *  preparation for a mode switch.
 *
 *  @param thr Thread to prepare.
 *
 *  @return Void.
 **/
void init_stack(thread_t *thr)
{
  /* Push the arguments on the kernel stack */
  void *esp = &thr->kstack[KSTACK_SIZE];
  PUSH(esp, thr->sp);
  PUSH(esp, thr->pc);
  PUSH(esp, 0);

  /* Set the correct program counter and stack pointer */
  thr->pc = half_dispatch;
  thr->sp = esp;

  return;
}

