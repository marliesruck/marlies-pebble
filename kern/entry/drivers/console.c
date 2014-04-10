/** @file console.c
 *
 *  @brief A basic console driver library.
 *
 *  @author Enrique Naudon (esn)
 *
 *  @bugs No known bugs.
 **/

#include "console_internal.h"
#include "cursor.h"

#include <mutex.h>
#include <stddef.h>
#include <string.h>
#include <video_defines.h>
#include <x86/asm.h>


/** @var console_lock
 *  @brief Serializes access to the console.
 **/
mutex_s console_lock = MUTEX_INITIALIZER(console_lock);

/* The logical cursor */
cursor_s cursor = CRS_INITIALIZER(CONSOLE_HEIGHT, CONSOLE_WIDTH);

/* The console */
console_s console = CONSOLE_INITIALIZER(FGND_LGRAY|BGND_BLACK,
                                        CONSOLE_MEM_BASE);


/*************************************************************************/
/* External Interface                                                    */
/*************************************************************************/

/** @brief Prints character ch with the specified color
 *         at position (row, col).
 *
 *  If any argument is invalid, the function has no effect.
 *
 *  @param row The row in which to display the character.
 *  @param col The column in which to display the character.
 *  @param ch The character to display.
 *  @param color The color to use to display the character.
 *
 *  @return Void.
 **/
void draw_char(int row, int col, int ch, int color)
{
  if (!valid_row(row)) return;
  else if (!valid_col(col)) return;
  else if (!valid_char(ch)) return;
  else if (!valid_color(color)) return;

  console.array[row][col] = PACK_CHAR_COLOR(ch, color);

  return;
}

/** @brief Returns the character displayed at position (row, col).
 *
 *  @param row Row of the character.
 *  @param col Column of the character.
 *
 *  @return The character at (row, col), or an integer error code less than
 *          0 if either row or col are invalid.
 **/
char get_char(int row, int col)
{
  if (!valid_row(row)) return -1;
  else if (!valid_col(col)) return -1;
  return UNPACK_CHAR(console.array[row][col]);
}

/** @brief Changes the foreground and background color of future characters
 *         printed on the console.
 *
 *  If the color code is invalid, the function has no effect.  This
 *  function DOES NOT change the color of what is already on the console.
 *
 *  @param color The new color code.
 *
 *  @return 0 on success, or an integer error code less than 0 if color
 *          code is invalid.
 **/
int set_term_color(int color)
{
  /* Check we were given a valid color */
  if (!valid_color(color)) return -1;

  /* Change the color */
  mutex_lock(&console_lock);
  console.color = color;
  mutex_unlock(&console_lock);

  return 0;
}

/** @brief Writes the current foreground and background color of the
 *         console into the argument color.
 *
 *  If color is NULL, this function has no effect.
 *
 *  @param color The address to which the color will be written.
 *
 *  @return Void.
 **/
void get_term_color(int *color)
{
  if (color == NULL) return;
  *color = console.color;
  return;
}

/** @brief Sets the position of the cursor to the position (row, col).
 *
 *  This function is a protected version of set_cursor_raw(...); see that
 *  for details.
 *
 *  @param row The new row for the cursor.
 *  @param col The new column for the cursor.
 *
 *  @return 0 on success, or integer error code less than 0 if cursor
 *          location is invalid.
 **/
int set_cursor(int row, int col)
{
  int ret;

  mutex_lock(&console_lock);
  ret = set_cursor_raw(row, col);
  mutex_unlock(&console_lock);

  return ret;
}

/** @brief Writes the current position of the cursor into the arguments row
 *         and col.
 *
 *  If either row or col are NULL, this function has no effect.
 *
 *  @param row The address to which the current cursor row will be written.
 *  @param col The address to which the current cursor column will be
 *              written.
 *
 *  @return Void.
 **/
void get_cursor(int *row, int *col)
{
  mutex_lock(&console_lock);
  crs_get_coords(&cursor, row, col);
  mutex_unlock(&console_lock);
  return;
}

/** @brief Shows the cursor.
 *  
 *  If the cursor is already shown, the function has no effect.
 *
 *  @return Void.
 **/
void show_cursor()
{
  int off;

  /* Set visibility first, or set_cursor(...) fails */
  crs_reveal(&cursor);
  off = crs_get_offset(&cursor);
  set_crtc(off);
  return;
}

/** @brief Hides the cursor.
 *
 *  Subsequent calls to putbytes do not cause the
 *  cursor to show again.
 *
 *  @return Void.
 **/
void hide_cursor()
{
  /* We call set_crtc(...) directly because we want to set the cursor
   * beyond the limits of the console; set_cursor(...) does not allow this.
   */
  set_crtc(CONSOLE_LIMIT);
  crs_conseal(&cursor);
  return;
}

/** @brief Clears the entire console.
 *
 * The cursor is reset to the first row and column
 *
 *  @return Void.
 **/
void clear_console(void)
{
  int r, c;
  console_foreach(r,c)
    console.array[r][c] = PACK_CHAR_COLOR(' ', console.color);
  set_cursor_raw(0,0);
  return;
}

/** @brief Prints character ch at the current location of the cursor.
 *
 *  If the character is a newline ('\n'), the cursor is be moved to the
 *  beginning of the next line (scrolling as needed).  If the character is
 *  a carriage return ('\r'), the cursor is immediately reset to the
 *  beginning of the current line, causing any future output to overwrite
 *  any existing output on the line--note that existing characters are NOT
 *  erased.  If backspace ('\b') occurs at the beginning of a row, this
 *  function has no effect; otherwise the previous character is erased.
 *
 *  @param ch The character to print.
 *
 *  @return The input character.
 **/
int putbyte(char ch)
{
  int row, col, color;

  if (!valid_char(ch) && !valid_ctrl(ch)) return ch;

  crs_get_coords(&cursor, &row, &col);
  get_term_color(&color);

  switch (ch)
  {
  case '\n':  /* Newline */
    if (set_cursor_raw(row+1, 0)) newline();
    break;
  case '\r':  /* Carriage return */
    set_cursor_raw(row, 0);
    break;
  case '\b':  /* Backspace */
    if (col > 0) {
      col = col-1;
      draw_char(row, col, ' ', color);
      set_cursor_raw(row, col);
    }
    break;
  default:    /* "Regular" characters */
    draw_char(row, col, ch, color);
    if (inc_cursor(1)) newline();
    break;
  }

  return ch;
}

/** @brief Prints the string s, starting at the current location of the
 *         cursor.
 *
 *  If the string is longer than the current line, the string fills up the
 *  current line and then continues on the next line. If the string exceeds
 *  available space on the entire console, the screen scrolls up one line,
 *  and then the string continues on the new line.  If '\n', '\r', and '\b'
 *  are encountered within the string, they are handled as per putbyte. If
 *  any character in the string is unprintable, this function has no
 *  effect.If len is not a positive integer or s is null, the function has
 *  no effect.
 *
 *  @param s The string to be printed.
 *  @param len The length of the string s.
 *
 *  @return Void.
 **/
void putbytes(const char *s, int len)
{
  int i;

  if (s == NULL) return;
  else if (len < 0) return;

  /* Validate input string */
  for (i = 0; i < len; ++i)
    if (!valid_char(s[i]) && !valid_ctrl(s[i])) return;

  /* Write to the console */
  mutex_lock(&console_lock);
  for (i = 0; i < len; ++i) putbyte(s[i]);
  mutex_unlock(&console_lock);

  return;
}


/*************************************************************************/
/* Internal helper functions                                             */
/*************************************************************************/

/** @brief Sets the position of the cursor to the position (row, col).
 *
 *  Subsequent calls to putbyte(s) should cause the console output to begin
 *  at the new position. If the cursor is currently hidden, a call to
 *  set_cursor() does not show the cursor.
 *
 *  @param row The new row for the cursor.
 *  @param col The new column for the cursor.
 *
 *  @return 0 on success, or integer error code less than 0 if cursor
 *          location is invalid.
 **/
int set_cursor_raw(int row, int col)
{
  unsigned short off;

  if (!valid_row(row)) return -1;
  else if (!valid_col(col)) return -1;

  /* Only move the hardware cursor if it's visible */
  off = coords2offset(row, col);
  if (crs_isvisible(&cursor))
    set_crtc(off);

  crs_set_offset(&cursor, off);

  return 0;
}

/** @brief Translates a coordinate position into an offset position.
 *
 *  @param row The row portion of the position to translate
 *  @param col The col portion of the position to translate
 *
 *  @return The equivalent offset position.
 **/
int coords2offset(int row, int col)
{
  return row*CONSOLE_WIDTH + col;
}

/** @brief Translates an offset position into a coordinate position.
 *
 *  @param off The offset to translate
 *  @param row The address at which to store the row
 *  @param col The address at which to store the column
 *
 *  @return Void.
 **/
void offset2coords(int off, int *row, int *col)
{
  *row = off/CONSOLE_WIDTH;
  *col = off - ((*row)*CONSOLE_WIDTH);
  return;
}

/** @brief Move the hardware cursor to the specified offset.
 *
 *  @param off The offset to which we're moving the cursor
 *
 *  @return Void.
 **/
void set_crtc(int off)
{
  outb(CRTC_IDX_REG, CRTC_CURSOR_LSB_IDX);
  outb(CRTC_DATA_REG, off & 0xFF);

  outb(CRTC_IDX_REG, CRTC_CURSOR_MSB_IDX);
  outb(CRTC_DATA_REG, off >> 8);

  return;
}

/** @brief Increment the cursor position.
 *
 *  @param n The number of spaces to move the cursor
 *
 *  @return Void.
 **/
int inc_cursor(int n)
{
  int old_r, old_c, new_r, new_c;

  crs_get_coords(&cursor, &old_r, &old_c);

  new_c = (old_c + n) % CONSOLE_WIDTH;
  new_r = old_r + (old_c + n) / CONSOLE_WIDTH;

  return set_cursor_raw(new_r, new_c);
}

/** @brief Scroll the console.
 *
 *  If the number of lines exceeds the console height, this function has no
 *  effect.
 *
 *  @param lines The number of lines to scroll
 *
 *  @return Void.
 **/
void scroll_console(int lines)
{
  int r, c;

  if (lines > CONSOLE_HEIGHT) return;

  /* Scroll lines
   * FIXME: I'm undefined
   */
  memcpy(console.array[0], console.array[lines],
         2*(CONSOLE_LIMIT - lines*CONSOLE_WIDTH));

  /* Clear the last lines rows
   * FIXME: I'm (very) ugly
   */
  console_region_foreach(r, c, CONSOLE_HEIGHT - lines,
                         CONSOLE_HEIGHT, 0, CONSOLE_WIDTH)
  {
    console.array[r][c] = PACK_CHAR_COLOR(' ', console.color);
  }
  return;
}

/** @brief Scroll the console and set the cursor to the first column of the
 *         new line.
 *
 *  @return Void.
 **/
void newline(void)
{
  int row, col;
  crs_get_coords(&cursor, &row, &col);
  scroll_console(1);
  set_cursor_raw(CONSOLE_HEIGHT-1, 0);
  return;
}

