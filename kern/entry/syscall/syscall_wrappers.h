/** @file syscall_wrappers.h
 *
 *  @brief Declares assembly wrapper for the various syscalls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#ifndef __SYSCALL_WRAPPERS_H__
#define __SYSCALL_WRAPPERS_H__

int asm_sys_fork(void);
void asm_sys_exec(void);
void asm_sys_set_status(void);
void asm_sys_vanish(void);
int asm_sys_wait(void);
void asm_sys_task_vanish(void);
int asm_sys_gettid(void);
void asm_sys_yield(void);
unsigned int asm_sys_get_ticks(void);
void asm_sys_sleep(void);
int asm_sys_new_pages(void);
int asm_sys_remove_pages(void);
char asm_sys_getchar(void);
int asm_sys_readline(void);
int asm_sys_print(void);
int asm_sys_set_term_color(void);
int asm_sys_set_cursor_pos(void);
int asm_sys_get_cursor_pos(void);
void asm_sys_halt(void);
int asm_sys_readfile(void);
void asm_sys_swexn(void);
void asm_sys_misbehave(void);
int asm_sys_deschedule(void);
int asm_sys_make_runnable(void);

#endif /* __SYSCALL_WRAPPERS_H__ */
