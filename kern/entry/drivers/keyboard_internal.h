/** @file keyboard_internal.h 
 *
 *  @brief Contains internal declarations and macros for the keyboard
 *         driver.
 *
 *  @author Enrique Naudon (esn)
 *
 *  @bugs No known bugs.
 */

#ifndef __KEYBOARD_INTERNAL_H__
#define __KEYBOARD_INTERNAL_H__

#define KBD_BUFFER_SIZE 64

/** @brief The keyboard buffer.
 **/
struct keyboard_buffer {
  int r, w;                       /* Read/write indicies */
  int count;                      /* Num elements in buffer */
  char buffer[KBD_BUFFER_SIZE];   /* The buffer itself */
};
typedef struct keyboard_buffer kbd_buffer;

/** @brief Static initializer for the keyboard buffer.
 *
 *  @return A statically allocated keyboard buffer.
 **/
#define KBD_BUFFER_INITIALIZER() {  \
  (int) 0, (int) 0,                 \
  (int) 0,                          \
  (char [KBD_BUFFER_SIZE]) {0}      \
}

/** @brief Performs a wrapping increment.
 *
 *  Increments i, wrapping when i hits the buffer size.
 *
 *  @param i The value to increment
 *
 *  @return The incremented value
 **/
#define MODINC(i) (((i) + 1) % KBD_BUFFER_SIZE)

/* Read and write from the scancode buffer */
static void __buffer_write(char scancode);
static int __buffer_read(char *scancode);

#endif
