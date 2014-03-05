/** @file scwrappers.S
 *
 *  @brief Implements assembly wrapper for the various system calls.
 *
 *  See /kern/asm/scwrappers.h for the expanded SYSCALL macros.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#include "sc_utils.h"

/* Special */
.extern sys_fork

.global asm_sys_fork
asm_sys_fork:

  # Save callee-saved registers for the child
  push %esi
  push %edi
  push %ebx

  # Save data segments
  push %ds                        # Store DS data segment
  push %es                        # Store ES data segment
  push %fs                        # Store FS data segment
  push %gs                        # Store GS data segment

  # Push the %esp as an argument
  push %esp

  call sys_fork

  # Clean up stack
  pop %ecx
  
  # Restore data segments
  pop %gs                         # Restore GS data segment
  pop %fs                         # Restore FS data segment
  pop %es                         # Restore ES data segment
  pop %ds                         # Restore DS data segment

  # Restore callee-saved registers
  pop %ebx
  pop %edi
  pop %esi

  # To userland!
  iret

/*************************************************************************
 *  Life cycle
 *************************************************************************/

N_ARY_SYSCALL sys_exec,$2
UNARY_SYSCALL sys_set_status
NULLARY_SYSCALL sys_vanish
UNARY_SYSCALL sys_wait
UNARY_SYSCALL sys_task_vanish


/*************************************************************************
 *  Thread management
 *************************************************************************/

NULLARY_SYSCALL sys_gettid
UNARY_SYSCALL sys_yield
UNARY_SYSCALL sys_deschedule
UNARY_SYSCALL sys_make_runnable
NULLARY_SYSCALL sys_get_ticks
UNARY_SYSCALL sys_sleep


/*************************************************************************
 *  Memory management
 *************************************************************************/

N_ARY_SYSCALL sys_new_pages,$2
UNARY_SYSCALL sys_remove_pages


/*************************************************************************
 *  Console I/O
 *************************************************************************/

/* Maybe we do something special for character returns?? */
NULLARY_SYSCALL sys_getchar
N_ARY_SYSCALL sys_readline,$2
N_ARY_SYSCALL sys_print,$2
UNARY_SYSCALL sys_set_term_color
N_ARY_SYSCALL sys_set_cursor_pos,$2
N_ARY_SYSCALL sys_get_cursor_pos,$2


/*************************************************************************
 *  Miscellaneous
 *************************************************************************/

NULLARY_SYSCALL sys_halt
N_ARY_SYSCALL sys_readfile,$4
N_ARY_SYSCALL sys_swexn,$4


/*************************************************************************
 *  "Special"
 *************************************************************************/

UNARY_SYSCALL sys_misbehave

