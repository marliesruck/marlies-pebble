/** @file syscalls.c
 *
 *  @brief Implements our system calls.
 *
 *  Even though any two system calls might do two VERY different things, we
 *  decided to implement them all in one file so that our user-facing API
 *  is in one place.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <simics.h>

#include <process.h>
#include <thread.h>
#include <ctx_switch.h>
#include <idt.h>
#include <syscall_int.h>
#include "syscall_wrappers.h"
#include "sc_utils.h"

/* Internal fork helper routines */
#include "fork_i.h"

#include <string.h>

/** @brief Installs our system calls.
 *
 *  @return Void.
 **/
void install_sys_handlers(void)
{
  install_trap_gate(FORK_INT, asm_sys_fork, IDT_USER_DPL);
  install_trap_gate(EXEC_INT, asm_sys_exec, IDT_USER_DPL);
  install_trap_gate(SET_STATUS_INT, asm_sys_set_status, IDT_USER_DPL);
  install_trap_gate(VANISH_INT, asm_sys_vanish, IDT_USER_DPL);
  install_trap_gate(WAIT_INT , asm_sys_wait, IDT_USER_DPL);
  install_trap_gate(TASK_VANISH_INT, asm_sys_task_vanish, IDT_USER_DPL);
  install_trap_gate(GETTID_INT, asm_sys_gettid, IDT_USER_DPL);
  install_trap_gate(YIELD_INT, asm_sys_yield, IDT_USER_DPL);
  install_trap_gate(DESCHEDULE_INT, asm_sys_deschedule, IDT_USER_DPL);
  install_trap_gate(GET_TICKS_INT, asm_sys_get_ticks, IDT_USER_DPL);
  install_trap_gate(SLEEP_INT , asm_sys_sleep, IDT_USER_DPL);
  install_trap_gate(NEW_PAGES_INT, asm_sys_new_pages, IDT_USER_DPL);
  install_trap_gate(REMOVE_PAGES_INT, asm_sys_remove_pages, IDT_USER_DPL);
  install_trap_gate(GETCHAR_INT, asm_sys_getchar, IDT_USER_DPL);
  install_trap_gate(READLINE_INT, asm_sys_readline, IDT_USER_DPL);
  install_trap_gate(PRINT_INT, asm_sys_print, IDT_USER_DPL);
  install_trap_gate(SET_TERM_COLOR_INT, asm_sys_set_term_color, IDT_USER_DPL);
  install_trap_gate(SET_CURSOR_POS_INT, asm_sys_set_cursor_pos, IDT_USER_DPL);
  install_trap_gate(GET_CURSOR_POS_INT, asm_sys_get_cursor_pos, IDT_USER_DPL);
  install_trap_gate(HALT_INT, asm_sys_halt, IDT_USER_DPL);
  install_trap_gate(READFILE_INT, asm_sys_readfile, IDT_USER_DPL);
  install_trap_gate(SWEXN_INT, asm_sys_swexn, IDT_USER_DPL);
  install_trap_gate(MISBEHAVE_INT, asm_sys_misbehave, IDT_USER_DPL);

  return;
}


/*************************************************************************
 *  Life cycle
 *************************************************************************/

int sys_fork(unsigned int esp)
{
  /* Copy the parent's page directory and tables */
  void *child_cr3 = mem_map_child();

  /* Initialize child tcb */
  thread_t *thread2 = init_child_tcb(child_cr3);

  int tid = thread2->tid;

  /* Compute the parent's esp offstack from its kstack base */
  unsigned int offset = esp - ((unsigned int) &curr->kstack);
  size_t len = ((unsigned int) &curr->kstack[KSTACK_SIZE]) - esp;

  /* Copy the parent's kstack */
  void *dest = &thread2->kstack[offset];
  memcpy(dest, (void *)esp, len);

  /* Set the child's pc to finish_fork and sp to its esp relative its own kstack */
  thread2->sp = &thread2->kstack[offset];
  thread2->pc = finish_fork;

  thrlist_enqueue(thread2,&naive_thrlist);

  /* Atomically insert child into runnable queue */
  return tid;
}

#include <loader.h>
#include <usr_stack.h>
#include <vm.h>
#include <malloc.h>
#include <string.h>
void mode_switch(void *entry_point, void *sp);
int sys_exec(char *execname, char *argvec[])
{
  char *execname_k, **argvec_k;
  int i, j;

  /* Copy execname from user-space */
  execname_k = malloc(strlen(execname) + 1);
  if (!execname_k) return -1;
  if (copy_from_user(execname_k, execname, strlen(execname) + 1)) {
    free(execname_k);
    return -1;
  }

  /* Copy argvec from user-space */
  for (i = 0; argvec[i] != NULL; ++i) continue;
  argvec_k = malloc((i + 1) * sizeof(char *));
  for (i = 0; argvec[i] != NULL; ++i)
  {
    argvec_k[i] = malloc(strlen(argvec[i]) + 1);
    if (copy_from_user(argvec_k[i], argvec[i], strlen(argvec[i]) + 1)) {
      free(execname_k);
      for (j = 0; j < i; ++j) free(argvec_k[j]);
      free(argvec_k);
      return -1;
    }
  }
  argvec_k[i] = NULL;

  /* Destroy the old address space; setup the new */
  vm_final(&curr->task_info->vmi);
  void *entry_point = load_file(&curr->task_info->vmi, execname_k);
  void *usr_sp = usr_stack_init(&curr->task_info->vmi, argvec_k);

  /* Free copied parameters*/
  for (j = 0; j < i; ++j) free(argvec_k[j]);
  free(argvec_k);
  free(execname_k);

  /* Execute the new program */
  mode_switch(entry_point, usr_sp);

  return 0;
}

void sys_set_status(int status)
{
  return;
}

void sys_vanish(void)
{
  return;
}

int sys_wait(int *status_ptr)
{
  return -1;
}

void sys_task_vanish(int status)
{
  return;
}


/*************************************************************************
 *  Thread management
 *************************************************************************/

int sys_gettid(void)
{
  return curr->tid;
}

int sys_yield(int pid)
{
  return -1;
}

int sys_deschedule(int *flag)
{
  return -1;
}

int sys_make_runnable(int pid)
{
  return -1;
}

unsigned int sys_get_ticks(void)
{
  return 0;
}

int sys_sleep(int ticks)
{
  return -1;
}


/*************************************************************************
 *  Memory management
 *************************************************************************/

int sys_new_pages(void *addr, int len)
{
  return -1;
}

int sys_remove_pages(void *addr)
{
  return -1;
}


/*************************************************************************
 *  Console I/O
 *************************************************************************/

/* REMOVED FOR DEBUGGING, REVERT THIS
char sys_getchar(void)
{
  return -1;
}
*/

int sys_readline(int size, char *buf)
{
  return -1;
}

int sys_print(int size, char *buf)
{
  return -1;
}

int sys_set_term_color(int color)
{
  return -1;
}

int sys_set_cursor_pos(int row, int col)
{
  return -1;
}

int sys_get_cursor_pos(int *row, int *col)
{
  return -1;
}


/*************************************************************************
 *  Miscellaneous
 *************************************************************************/

void sys_halt()
{
  return;
}

int sys_readfile(char *filename, char *buf, int count, int offset)
{
  return -1;
}

#include <ureg.h>
typedef void (*swexn_handler_t)(void *arg, ureg_t *ureg);
int sys_swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
  return -1;
}


/* "Special" */
void sys_misbehave(int mode)
{
  return;
}

