/** @file scwrappers.S
 *
 *  @brief Implements assembly wrapper for the various system calls.
 *
 *  See /kern/asm/scwrappers.h for the expanded SYSCALL macros.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#include <sc_utils.h>


/*************************************************************************
 *  Life cycle
 *************************************************************************/

FORK_SYSCALL sys_fork
FORK_SYSCALL sys_thread_fork
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

