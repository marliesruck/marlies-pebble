/** @file cursor.h 
 *
 *  @brief Contains declarations for our logical cursor implementation.
 *
 *  A cursor is simply a point in a 2-dimentional space, along with a flag
 *  indicating it's visibility.
 *
 *  @author Enrique Naudon (esn)
 *
 *  @bugs No known bugs.
 */
#ifndef __CURSOR_H__
#define __CURSOR_H__

#include <types.h>


/** @brief A struct representing the logical cursor.
 **/
struct cursor {
  int off;              /* The current offset of the cursor */
  size_t x_lim, y_lim;  /* The x/y limits for the cursor */
  int vis;              /* Cursor visibility flag */
};
typedef struct cursor cursor_s;

/** @brief Cursor visiblity flags.
 **/
enum crs_visibility_e {
  CRS_VISIBLE,
  CRS_HIDDEN,
};

/** @brief Static initializer for cursors.
 *
 *  @return A statically allocated cursor.
 **/
#define CRS_INITIALIZER(x, y) {   \
  (int) 0,                        \
  (size_t) x, (size_t) y,         \
  (int) CRS_VISIBLE               \
}

/** @brief Evaluates to true if the coordinates, x and y, are valid.
 *
 *  @param c The cursor we're working with.
 *  @param x The x value to validate.
 *  @param y The y value to validate.
 *
 *  @return 1 if valid, 0 otherwise.
 **/
#define crs_valid_coords(c, x, y)   \
  ( ( (y) < (c)->y_lim && (y) >= 0 ) && ( (x) < (c)->x_lim && (x) >= 0 ) )

/** @brief Evaluates to true if the offset, o, is valid.
 *
 *  @param c The cursor we're working with.
 *  @param o The offset value to validate.
 *
 *  @return 1 if valid, 0 otherwise.
 **/
#define crs_valid_offset(c, o)  \
  ( (o) < ((c)->x_lim * (c)->y_lim) && (o) >= 0 )

/* Cursor initialization */
void crs_init(cursor_s *crs, int x, int y);

/* Cursor location */
int crs_set_offset(cursor_s *crs, int off);
int crs_get_offset(cursor_s *crs);
int crs_set_coords(cursor_s *crs, int x, int y);
int crs_get_coords(cursor_s *crs, int *x, int *y);

/* Cursor visibility */
inline void crs_reveal(cursor_s *crs);
inline void crs_conseal(cursor_s *crs);
inline int crs_isvisible(cursor_s *crs);

#endif

