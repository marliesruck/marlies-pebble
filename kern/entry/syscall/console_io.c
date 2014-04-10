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

/** @brief Retrieve a character from the keyboard.
 *
 *  If no character exists in the input stream, the caller blocks
 *  until a character becomes available.  Furthermore, if other threads are
 *  waiting for input, the caller will block and wait their turn.
 *
 *  @return A character from the input stream.
 **/
char sys_getchar(void)
{
  char ch;

  ch = kbd_getchar();
  if (ch == -1) return -1;

  return ch;
}

/** @brief Retrieve a new line-terminated line from the keyboard.
 *
 *  If no line exists in the input stream, the caller blocks until a line
 *  becomes available.  Furthermore, if other threads are waiting for
 *  input, the caller will block and wait their turn.  If there is not
 *  enough room in the buffer for the entire line, only size bytes are
 *  copied into the buffer.
 *
 *  @param size The size of the user buffer.
 *  @param buf A character buffer to copy data into.
 *
 *  @return The number of bytes copied into the buffer.
 **/
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

/** @brief Prints a string to the console.
 *
 *  If any part of buf is in invalid memory or the system cannot allocate
 *  memory to copy buf into kernel space, sys_print(...) will fail with an
 *  error code.
 *
 *  @param size The number of bytes to print.
 *  @param buf A pointer to the start of the bytes to print.
 *
 *  @return 0 on success, or a negative integer erro code on failure.
 **/
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

/** @brief Set the color of the console.
 *
 *  @return 0 on success, or a negative integer erro code on failure.
 **/
int sys_set_term_color(int color)
{
  return set_term_color(color);
}

/** @brief Set the console cursor position.
 *
 *  @return 0 on success, or a negative integer erro code on failure.
 **/
int sys_set_cursor_pos(int row, int col)
{
  return set_cursor(row, col);
}

/** @brief Get the console cursor position.
 *
 *  If either row or col are in invalid memory, sys_get_cursor_pos(...)
 *  will return an error code.
 *
 *  @return 0 on success, or a negative integer erro code on failure.
 **/
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

