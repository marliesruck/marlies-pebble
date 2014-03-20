/** @file sc_utils.c
 *
 *  @brief Implements various system call-related utility functions.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <sc_utils.h>
#include "syscall_wrappers.h"

/* Pebble includes */
#include <idt.h>
#include <syscall_int.h>

/* Libc includes */
#include <assert.h>
#include <string.h>

/** @brief Safely invoke kernel system call handlers.
 *
 *  After validating the system call arguments, we invoke the handler and
 *  pass the arguments along.  For simplicity, we assume that all system
 *  call handlers return an unsigned 32-bit integer; it is the caller's
 *  responsibility to ignore void values.
 *
 *  @param func The system call handler.
 *  @param args The arguments to the handler.
 *  @param arity The arity of the handler.
 *
 *  @return Whatever the sytem call handler returns.
 **/
int sc_validate_argp(void *argp, int arity)
{
  return 0;
}

/** @brief Safely copy data from user-space.
 *
 *  TODO: This isn't actually safe; make it so.
 *
 *  @param dst The destionation pointer.
 *  @param src The (user-space) source pointer.
 *  @param bytes The number of bytes to copy.
 *
 *  @return 0 on success; a negative integer error code on failure.
 **/
int copy_from_user(char *dst, const char *src, size_t bytes)
{
  memcpy(dst, src, bytes);
  return 0;
}

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
  install_trap_gate(MAKE_RUNNABLE_INT, asm_sys_make_runnable, IDT_USER_DPL);
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

