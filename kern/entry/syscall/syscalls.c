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

#include <idt.h>
#include <syscall_int.h>
#include "syscall_wrappers.h"

/** @brief Installs our fault handlers.
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

int sys_fork(void)
{
  return -1;
}

int sys_exec(char *execname, char *argvec[])
{
  return -1;
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
  return my_pcb.my_tcb.tid;
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

int sys_new_pages(void * addr, int len)
{
  return -1;
}

int sys_remove_pages(void * addr)
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
