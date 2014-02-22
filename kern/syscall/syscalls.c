/** @file syscalls.S
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
  return -1;
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

char sys_getchar(void)
{
  return -1;
}

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

