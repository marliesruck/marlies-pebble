/** @file cursor.c 
 *
 *  @brief Contains an implementation of a generic logical cursor.
 *
 *  @author Enrique Naudon (esn)
 *
 *  @bugs No known bugs.
 */

#include "cursor.h"

#include <stddef.h>


/** @brief Initialize the cursor.
 *
 *  @param crs The cursor to initialize.
 *  @param x The maximum x value for the cursor.
 *  @param y The maximum y value for the cursor.
 *
 *  @return Void.
 */
void crs_init(cursor_s *crs, int x, int y)
{
  crs->off = 0;
  crs->x_lim = x;
  crs->y_lim = y;
  crs->vis = CRS_VISIBLE;
  return;
}

/** @brief Set the cursor's offset.
 *
 *  @param crs The cursor whose offset we modify.
 *  @param off The new offset.
 *
 *  @return 0 on success, or a negative integer error code on failure.
 */
int crs_set_offset(cursor_s *crs, int off)
{
  if (!crs_valid_offset(crs, off)) return -1;
  crs->off = off;
  return 0;
}

/** @brief Get the cursor's offset.
 *
 *  @param crs The cursor whose offset we want.
 *
 *  @return The cursor's current offset.
 */
inline int crs_get_offset(cursor_s *crs)
{
  return crs->off;
}

/** @brief Set the cursor's coordinates.
 *
 *  @param crs The cursor whose coordinates we modify.
 *  @param x The new x position.
 *  @param y The new y position.
 *
 *  @return 0 on success, or a negative integer error code on failure.
 */
int crs_set_coords(cursor_s *crs, int x, int y)
{
  if (!crs_valid_coords(crs, x, y)) return -1;
  crs->off = x*crs->y_lim + y;
  return 0;
}

/** @brief Get the cursor's coordinates.
 *
 *  @param crs The cursor whose coordinates we want.
 *  @param x The address to store the current x position.
 *  @param y The address to store the current y position.
 *
 *  @return 0 on success, or a negative integer error code on failure.
 */
int crs_get_coords(cursor_s *crs, int *x, int *y)
{
  if (x == NULL || y == NULL) return -1;
  *x = crs->off / crs->y_lim;
  *y = crs->off - ((*x) * crs->y_lim);
  return 0;
}

/** @brief Set the cursor to visible.
 *
 *  @param crs The cursor to make visible.
 *
 *  @return Void.
 */
inline void crs_reveal(cursor_s *crs)
{
  crs->vis = CRS_VISIBLE;
  return;
}

/** @brief Set the cursor to invisible.
 *
 *  @param crs The cursor to make invisible.
 *
 *  @return Void.
 */
inline void crs_conseal(cursor_s *crs)
{
  crs->vis = CRS_HIDDEN;
  return;
}

inline int crs_isvisible(cursor_s *crs)
{
  return crs->vis == CRS_VISIBLE;
}

