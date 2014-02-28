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

LONG_INTERRUPT sys_fork
LONG_INTERRUPT sys_exec
VOID_INTERRUPT sys_set_status
VOID_INTERRUPT sys_vanish
LONG_INTERRUPT sys_wait
VOID_INTERRUPT sys_task_vanish


/*************************************************************************
 *  Thread management
 *************************************************************************/

LONG_INTERRUPT sys_gettid
LONG_INTERRUPT sys_yield
LONG_INTERRUPT sys_deschedule
LONG_INTERRUPT sys_make_runnable
LONG_INTERRUPT sys_get_ticks
LONG_INTERRUPT sys_sleep


/*************************************************************************
 *  Memory management
 *************************************************************************/

LONG_INTERRUPT sys_new_pages
LONG_INTERRUPT sys_remove_pages


/*************************************************************************
 *  Console I/O
 *************************************************************************/

/* Maybe we do something special for character returns?? */
LONG_INTERRUPT sys_getchar
LONG_INTERRUPT sys_readline
LONG_INTERRUPT sys_print
LONG_INTERRUPT sys_set_term_color
LONG_INTERRUPT sys_set_cursor_pos
LONG_INTERRUPT sys_get_cursor_pos


/*************************************************************************
 *  Miscellaneous
 *************************************************************************/

VOID_INTERRUPT sys_halt
LONG_INTERRUPT sys_readfile
LONG_INTERRUPT sys_swexn


/*************************************************************************
 *  "Special"
 *************************************************************************/

VOID_INTERRUPT sys_misbehave

