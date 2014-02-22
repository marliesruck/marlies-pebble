/** @file scwrappers.S
 *
 *  @brief Implements assembly wrapper for the various system calls.
 *
 *  See /kern/asm/intwrappers.h for the expanded INTERRUPT macro.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#include <asm/intwrappers.h>


/*************************************************************************
 *  Life cycle
 *************************************************************************/

INTERRUPT sys_fork
INTERRUPT sys_exec
INTERRUPT sys_set_status
INTERRUPT sys_vanish
INTERRUPT sys_wait
INTERRUPT sys_task_vanish


/*************************************************************************
 *  Thread management
 *************************************************************************/

INTERRUPT sys_gettid
INTERRUPT sys_yield
INTERRUPT sys_deschedule
INTERRUPT sys_make_runnable
INTERRUPT sys_get_ticks
INTERRUPT sys_sleep


/*************************************************************************
 *  Memory management
 *************************************************************************/

INTERRUPT sys_new_pages
INTERRUPT sys_remove_pages


/*************************************************************************
 *  Console I/O
 *************************************************************************/

INTERRUPT sys_getchar
INTERRUPT sys_readline
INTERRUPT sys_print
INTERRUPT sys_set_term_color
INTERRUPT sys_set_cursor_pos
INTERRUPT sys_get_cursor_pos


/*************************************************************************
 *  Miscellaneous
 *************************************************************************/

INTERRUPT sys_halt
INTERRUPT sys_readfile
INTERRUPT sys_swexn


/*************************************************************************
 *  "Special"
 *************************************************************************/

INTERRUPT sys_misbehave

