/** @file console_internal.h 
 *
 *  @brief Contains internal declarations and macros for the console
 *         driver.
 *
 *  @author Enrique Naudon (esn)
 *
 *  @bugs No known bugs.
 */
#ifndef __CONSOLE_INTERNAL_H__
#define __CONSOLE_INTERNAL_H__

#include <ctype.h>
#include <limits.h>
#include <video_defines.h>


/* Should be in video_defines.h, but make update is territorial */
#define CONSOLE_LIMIT (CONSOLE_WIDTH * CONSOLE_HEIGHT)
#define CONSOLE_TABWIDTH 8

/** @brief A struct representing the console.
 **/
struct console {
  char color;     /* Fore/Background color for future characters */
  short (*array)[CONSOLE_WIDTH];  /* Pointer to the console's memory */
};
typedef struct console console_s;

/** @brief Static initializer for a console struct.
 *
 *  @param color The default console color.
 *  @param base Pointer to the start of the console's memory.
 *
 *  @return A statically allocated console struct.
 **/
#define CONSOLE_INITIALIZER(color, base) {  \
  (char) (color),                           \
  (short (*)[CONSOLE_WIDTH]) (base)         \
}

/** @brief Packs a character and color together into a console short.
 *
 *  @param ch The character value to pack.
 *  @param color The color value to pack.
 *
 *  @return The packed short.
 **/
#define PACK_CHAR_COLOR(ch, color)  \
  ( (short) ( (((unsigned char) (color)) << 8) | ((unsigned char) (ch)) ) )

/** @brief Retrieves the character from a console short.
 *
 *  @param shrt The console short to unpack.
 *
 *  @return The unpacked character value.
 **/
#define UNPACK_CHAR(shrt) ((unsigned char) ((shrt) & 0xFF))

/** @brief Retrieves the color from a console short.
 *
 *  @param shrt The console short to unpack.
 *
 *  @return The unpacked color value.
 **/
#define UNPACK_COLOR(shrt) ((unsigned char) ((shrt) >> 8))

/** @brief Evaluates to true if the row, r, is valid.
 *
 *  @param r The row value to validate.
 *
 *  @return 1 if valid, 0 otherwise.
 **/
#define valid_row(r)    ( (r) < CONSOLE_HEIGHT && (r) >= 0 )

/** @brief Evaluates to true if the column, c, is valid.
 *
 *  @param c The column value to validate.
 *
 *  @return 1 if valid, 0 otherwise.
 **/
#define valid_col(c)    ( (c) < CONSOLE_WIDTH && (c) >= 0 )

/** @brief Evaluates to true if the offset, o, is valid.
 *
 *  @param o The offset value to validate.
 *
 *  @return 1 if valid, 0 otherwise.
 **/
#define valid_off(o)    ( (o) < CONSOLE_LIMIT && (o) >= 0 )

/** @brief Evaluates to true if the character, c, is valid.
 *
 *  We use isprint(...) to validate characters despite the fact that
 *  isprint(...) reports extended ASCII values as unprintable.  We do this
 *  for portability reasons--there are various implementations of 8-bit
 *  ASCII, and they are not all compatible.
 *
 *  @param c The character value to validate.
 *
 *  @return 1 if valid, 0 otherwise.
 **/
#define valid_char(c)   ( isprint((c)) )

/** @brief Evaluates to true if the character, c, is a valid control
 *         character.
 *
 *  The control characters we allow are newlines, carriage return and
 *  backspace('\n', '\r' and '\b', respectively.)
 *
 *  @param c The character value to validate.
 *
 *  @return 1 if valid, 0 otherwise.
 **/
#define valid_ctrl(c)   \
  ( (c) == '\n' || (c) == '\r' || (c) == '\b' )

/** @brief Evaluates to true if the color, c, is valid.
 *
 *  @param c The color value to validate.
 *
 *  @return 1 if valid, 0 otherwise.
 **/
#define valid_color(c)  ( (c) <= UCHAR_MAX )

/** @brief A convenient loop for a region of the console array.
 *
 *  @param r The "cursor" variable for rows.
 *  @param c The "cursor" variable for columns.
 *  @param startr The starting row value.
 *  @param endr The ending row value.
 *  @param startc The starting column value.
 *  @param endc The ending column value.
 **/
#define console_region_foreach(r,c, startr, endr, startc, endc)   \
  for (r = startr; r < endr; ++r)     \
    for (c = startc; c < endc; ++c)   \

/** @brief A convenient loop for the console array.
 *
 *  @param r The "cursor" variable for rows.
 *  @param c The "cursor" variable for columns.
 *  @param startr The starting row value.
 *  @param endr The ending row value.
 *  @param startc The starting column value.
 *  @param endc The ending column value.
 **/
#define console_foreach(r,c)              \
  for (r = 0; r < CONSOLE_HEIGHT; ++r)    \
    for (c = 0; c < CONSOLE_WIDTH; ++c)   \

/* Translate between offsets and coordinates */
static inline int coords2offset(int row, int col);
static inline void offset2coords(int off, int *row, int *col);

/* Move the hardware cursor to the specified offset */
static inline void set_crtc(int off);

/* Scroll the console by the specified number of lines */
static inline void scroll_console(int lines);

/* Increment the cursor n spaces, scrolling as needed */
static int inc_cursor(int n);

/* Scroll the console 1 line; set the cursor to column 0 */
static inline void newline(void);

#endif /* __CONSOLE_INTERNAL_H__ */
