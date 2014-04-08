/** @file console_io.c
 *
 *  @brief Implements our console-related system calls.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <sc_utils.h>

/* Pebbles includes */
#include <console.h>
#include <keyboard.h>

/* Libc includes */
#include <malloc.h>
#include <string.h>


/*************************************************************************
 *  Console I/O
 *************************************************************************/

char sys_getchar(void)
{
  char ch;

  ch = kbd_getchar();
  if (ch == -1) return -1;

  return ch;
}

#include <simics.h>
int sys_readline(int size, char *buf)
{
  char *buf_k;
  int len;
  
  /* Get a line from the console */
  buf_k = malloc(size * sizeof(char));
  if (!buf_k) return -1;

  len = kbd_getline(size, buf_k);

  /* Give it to the user */
  if (copy_to_user(buf, buf_k, len)) {
    free(buf_k);
    return -1;
  }

  /* Free and return */
  free(buf_k);
  return len;
}

int sys_print(int size, char *buf)
{
  char *buf_k;

  /* Copy buf_k from user-space */
  if (copy_from_user(&buf_k, buf, size)) {
    free(buf_k);
    return -1;
  }

  /* Write to the console */
  putbytes(buf_k, size);

  free(buf_k);
  return 0;
}

int sys_set_term_color(int color)
{
  return set_term_color(color);
}

int sys_set_cursor_pos(int row, int col)
{
  return set_cursor(row, col);
}

int sys_get_cursor_pos(int *row, int *col)
{
  int krow, kcol;

  /* Get cursor coords */
  get_cursor(&krow, &kcol);

  /* Copy coords to user */
  if ( copy_to_user((char *)&krow, (char *)row, sizeof(int))
       || copy_to_user((char *)&kcol, (char *)col, sizeof(int)) )
  {
    return -1;
  }

  return 0;
}

