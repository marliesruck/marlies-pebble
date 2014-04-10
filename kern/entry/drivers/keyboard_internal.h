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

#include <cvar.h>
#include <mutex.h>

#include <x86/keyhelp.h>


#define KBD_BUFFER_SIZE 64

/** @enum mutex_state
 *  @brief Flag values for mutex state.
 **/
enum kbd_state {
  KBD_AWAITING_NONE,    /**< No one controls the input stream **/
  KBD_AWAITING_CHAR,    /**< The owner of the stream wants a char **/
  KBD_AWAITING_LINE,    /**< The owner of the stream wants a line **/
};
typedef enum kbd_state kbd_state_e;


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
void buffer_write(char ch);
int buffer_read(char *chp);

/* Globals for retrieving a line of input */
char *getline_buf;
int getline_size;
int getline_count;

/* Write to the getline global buffer */
int update_getline_globals(char ch);


#endif /* __KEYBOARD_INTERNAL_H__ */

