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


/*************************************************************************
 *  Console I/O
 *************************************************************************/

char sys_getchar(void)
{
  char ch;
  while ((ch = kbd_getchar()) == -1) continue;
  return ch;
}

int sys_readline(int size, char *buf)
{
  int count = 0;
  char ch = '\0';

  while (count < size && ch != '\n')
  {
    /* Read characters from the keyboard */
    while ((ch = kbd_getchar()) == -1) continue;

    /* Write to user buffer and console*/
    buf[count] = ch;
    putbyte(ch);

    /* Inc/Decrement read count */
    if (ch == '\b')
      --count;
    else
      ++count;
  }

  return count;
}

int sys_print(int size, char *buf)
{
  char *buf_k;

  /* Copy buf_k from user-space */
  buf_k = malloc(size);
  if (!buf_k) return -1;
  if (copy_from_user(buf_k, buf, size)) {
    free(buf_k);
    return -1;
  }

  /* Write to the console */
  putbytes(buf, size);

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
  *row = krow;
  *col = kcol;

  return 0;
}

