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

NVOID_INTERRUPT sys_fork
NVOID_INTERRUPT sys_exec
VOID_INTERRUPT sys_set_status
VOID_INTERRUPT sys_vanish
NVOID_INTERRUPT sys_wait
VOID_INTERRUPT sys_task_vanish


/*************************************************************************
 *  Thread management
 *************************************************************************/

NVOID_INTERRUPT sys_gettid
NVOID_INTERRUPT sys_yield
NVOID_INTERRUPT sys_deschedule
NVOID_INTERRUPT sys_make_runnable
NVOID_INTERRUPT sys_get_ticks
NVOID_INTERRUPT sys_sleep


/*************************************************************************
 *  Memory management
 *************************************************************************/

NVOID_INTERRUPT sys_new_pages
NVOID_INTERRUPT sys_remove_pages


/*************************************************************************
 *  Console I/O
 *************************************************************************/

/* Maybe we do something special for character returns?? */
NVOID_INTERRUPT sys_getchar
NVOID_INTERRUPT sys_readline
NVOID_INTERRUPT sys_print
NVOID_INTERRUPT sys_set_term_color
NVOID_INTERRUPT sys_set_cursor_pos
NVOID_INTERRUPT sys_get_cursor_pos


/*************************************************************************
 *  Miscellaneous
 *************************************************************************/

VOID_INTERRUPT sys_halt
NVOID_INTERRUPT sys_readfile
NVOID_INTERRUPT sys_swexn


/*************************************************************************
 *  "Special"
 *************************************************************************/

VOID_INTERRUPT sys_misbehave

