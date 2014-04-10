/** @file keyboard.c
 *
 *  @brief Implements our keyboard driver code.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

/* Keyboard includes */
#include "keyboard_internal.h"

/* Pebble includes */
#include <interrupt_defines.h>
#include <x86/asm.h>


/** @var kbd_lock
 *  @brief Serializes access to the keyboard input stream.
 **/
mutex_s kbd_lock = MUTEX_INITIALIZER(kbd_lock);

/** @var kbd_wait
 *  @brief Allows threads to wait on input.
 **/
cvar_s kbd_wait = CVAR_INITIALIZER(kbd_wait);

/** @var kbd_state
 *  @brief Indicates the state of the keyboard.
 **/
kbd_state_e kbd_state = KBD_AWAITING_NONE;


/*************************************************************************/
/* External Interface                                                    */
/*************************************************************************/

/** @brief Retrieves the next character in the keyboard buffer.
 *
 *  If no character exists in the keyboard buffer, this function will block
 *  until a character becomes available.  If any other thread is already
 *  blocked waiting for input, the invoking thread will wait until those
 *  threads have revieved input.
 *
 *  @return The next character in the keyboard buffer.
 **/
int kbd_getchar(void)
{
  char ch;

  mutex_lock(&kbd_lock);
  kbd_state = KBD_AWAITING_CHAR;

  /* Grab a character */
  while (!buffer_read(&ch))
  {
    /* Don't drop the mutex; it's your turn */
    cvar_wait(&kbd_wait, NULL);
  }

  mutex_unlock(&kbd_lock);
  return ch;
}

/** @brief Retrieves the next line typed at the keyboard.
 *
 *  If no line of input exist in the keyboard buffer, this function will
 *  block until a line becomes available.  If any other thread is already
 *  blocked waiting for input, the invoking thread will wait until those
 *  threads have revieved input.
 *
 *  @param size The size of the user buffer.
 *  @param buf A character buffer to copy data into.
 *
 *  @return The number of bytes copied into the buffer.
 **/
int kbd_getline(int size, char *buf)
{
  int count;
  char ch;

  mutex_lock(&kbd_lock);
  kbd_state = KBD_AWAITING_LINE;

  /* Set globals for the interrupt handler */
  getline_buf = buf;
  getline_size = size;
  getline_count = 0;

  /* Get any characters in the buffer */
  while (!buffer_read(&ch)) {
    if (update_getline_globals(ch)) {
      mutex_unlock(&kbd_lock);
      return getline_count;
    }
  }

  /* Wait for characters, but don't drop the lock*/
  cvar_wait(&kbd_wait, NULL);

  /* Unlock and return the count */
  count = getline_count;
  mutex_unlock(&kbd_lock);
  return count;
}

/** @brief Writes a character into the keyboard buffer.
 *
 *  This function allows the caller to write arbitrary character values
 *  into the keyboard buffer.  This function is intended for use as a
 *  debugging tool.
 *
 *  @param ch The character to write into the buffer.
 *
 *  @return Void.
 **/
void kbd_putchar(char ch)
{
  buffer_write(ch);
  return;
}

/** @brief Handles keyboard interrupts.
 *
 *  Read a scancode from the keyboard, process it.  If someone is waiting
 *  for a line of input, write the character into their buffer and signal
 *  them if it is a newline or of their buffer is full.  Otherwise write
 *  the character into the keyboard buffer and, if someone is waiting for a
 *  character, signal them.
 *
 *  @return Void.
 **/
void kbd_int_handler(void)
{
  char ch, scancode;
  kh_type k;

  /* Grab the scancode */
  scancode = inb(KEYBOARD_PORT);

  /* Process the scancode */
  k = process_scancode(scancode);

  if (KH_HASDATA(k) && KH_ISMAKE(k))
  {
    ch = KH_GETCHAR(k);
    switch (kbd_state)
    {
    /* Someone wants a line */
    case KBD_AWAITING_LINE:
      if (update_getline_globals(ch)) {
        kbd_state = KBD_AWAITING_NONE;
        cvar_signal_raw(&kbd_wait);
      }
      break;

    /* Someone wants a character */
    case KBD_AWAITING_CHAR:
      buffer_write(ch);
      kbd_state = KBD_AWAITING_NONE;
      cvar_signal_raw(&kbd_wait);
      break;

    /* No one is waiting */
    case KBD_AWAITING_NONE:
    default:
      buffer_write(ch);
      break;
    }
  }

  /* Ack the interrupt */
  outb(INT_CTL_PORT, INT_ACK_CURRENT);
  return;
}


/*************************************************************************/
/* Internal helper functions                                             */
/*************************************************************************/

/* The keyboard buffer and it's lock */
kbd_buffer buff = KBD_BUFFER_INITIALIZER();

/** @brief Write a scancode to the keyboard buffer.
 *
 *  Write the specified scancode into the keyboard's buffer.  If there is
 *  no room in the keyboard buffer, overwrite the oldest scancode.
 *
 *  @param ch The character to write into the buffer.
 *
 *  @return Void.
 **/
void buffer_write(char ch)
{
  disable_interrupts();

  /* Write into the buffer */
  buff.buffer[buff.w] = ch;
  buff.w = MODINC(buff.w);

  /* Increment read as needed */
  if (buff.count >= KBD_BUFFER_SIZE)
    buff.r = MODINC(buff.r);

  /* Increment the buffer count */
  ++buff.count;
  enable_interrupts();
  return;
}

/** @brief Read a scancode from the keyboard buffer.
 *
 *  We return the next scancode in the buffer at the address specified by
 *  scancode.  The return value indicates the number of lost (i.e.
 *  overwritten) entries since the last read.
 *
 *  NOTE: we disable interrupts to prevent the keyboard interrupt handler
 *  from writing while we read.
 *
 *  @param ch Destination pointer for read character.
 *
 *  @return 0 if a character was written to chp; or a negative integer
 *  error code if the buffer is empty
 **/
int buffer_read(char *chp)
{
  disable_interrupts();

  /* Check for an empty buffer */
  if (buff.count == 0) {
    enable_interrupts();
    return -1;
  }

  /* Read from the buffer */
  *chp = buff.buffer[buff.r];
  buff.r = MODINC(buff.r);

  /* Decrement the count */
  --buff.count;
  enable_interrupts();

  return 0;
}

/** @brief Place a character into the getline global buffer.
 *
 *  We also update the getline count global as well.  When the global
 *  buffer becomes full, return an error code.
 *
 *  @param k The kh_type to copy into the buffer.
 *
 *  @return 0 on success; a negative integer error code if the buffer is
 *  full; and 1 when a newline is encountered.
 **/
#include <console.h>
int update_getline_globals(char ch)
{
  /* Write the character to buf and the console */
  getline_buf[getline_count] = ch;
  putbyte(ch);

  /* Inc/Decrement read count */
  if (ch == '\b')
    getline_count = (getline_count > 0)
                    ? getline_count - 1
                    : getline_count;
  else
    ++getline_count;

  /* Let the caller know we can do no more */
  if (getline_count >= getline_size) return -1;
  if (ch == '\n') return 1;

  return 0;
}

