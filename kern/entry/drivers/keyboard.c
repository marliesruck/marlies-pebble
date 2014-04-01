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

mutex_s kbd_lock = MUTEX_INITIALIZER(kbd_lock);
cvar_s kbd_wait = CVAR_INITIALIZER(kbd_wait);
kbd_state_e kbd_state = KBD_AWAITING_NONE;


/*************************************************************************/
/* External Interface                                                    */
/*************************************************************************/

/** @brief Returns the next character in the keyboard buffer.
 *
 *  Search the keyboard's scancode buffer for a valid character-key press
 *  event; return the corresponding character.  If no such event is found,
 *  return -1.
 *
 *  NOTE: This function does not block if there are no characters in the
 *  keyboard buffer
 *
 *  @return The next character in the keyboard buffer, or -1 if the
 *          keyboard buffer is currently empty.
 **/
char getchar_buf;
int kbd_getchar(void)
{
  kh_type k;

  mutex_lock(&kbd_lock);
  kbd_state = KBD_AWAITING_CHAR;

  while (__buffer_read(&k) == -1)
    cvar_wait(&kbd_wait, NULL);

  mutex_unlock(&kbd_lock);
  return KH_GETCHAR(k);
}


#include <console.h>
#include <simics.h>
char *getline_buf;
int getline_size;
int getline_count;

int update_getline_globals(kh_type k)
{
  char ch;

  ch = KH_GETCHAR(k);

  /* Write the character to buf and the console */
  getline_buf[getline_count] = ch;
  putbyte(ch);

  /* Inc/Decrement read count */
  if (ch == '\b')
    --getline_count;
  else
    ++getline_count;

  /* Let the caller know we can do no more */
  if (getline_count >= getline_size || ch == '\n')
    return 1;

  return 0;
}

int kbd_getline(int size, char *buf)
{
  int count;

  mutex_lock(&kbd_lock);
  kbd_state = KBD_AWAITING_LINE;

  /* Set globals for the interrupt handler */
  getline_buf = buf;
  getline_size = size;
  getline_count = 0;

  /* Wait for characters */
  cvar_wait(&kbd_wait, NULL);

  /* For returning */
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
 *  @param scancode The scancode to write into the buffer
 *
 *  @return Void.
 **/
void kbd_putchar(kh_type k)
{
  __buffer_write(k);
  return;
}

/** @brief Handles keyboard interrupts.
 *
 *  Simply read a scancode from the keyboard and write it into the keyboard
 *  buffer.  All post-processing is saved for later (i.e. not in the
 *  context of an interrupt).
 *
 *  @return Void.
 **/
void kbd_int_handler(void)
{
  char scancode;
  kh_type k;

  scancode = inb(KEYBOARD_PORT);

  k = process_scancode(scancode);
  if (KH_HASDATA(k) && KH_ISMAKE(k))
  {
    switch (kbd_state)
    {
    /* Someone wants a line */
    case KBD_AWAITING_LINE:
      if (update_getline_globals(k)) {
        kbd_state = KBD_AWAITING_NONE;
        cvar_signal(&kbd_wait);
      }
      break;

    /* Someone wants a character */
    case KBD_AWAITING_CHAR:
      __buffer_write(k);
      kbd_state = KBD_AWAITING_NONE;
      cvar_signal(&kbd_wait);
      break;

    /* No one is waiting */
    case KBD_AWAITING_NONE:
    default:
      __buffer_write(k);
      break;
    }
  }

  /* We don't want nested keyboard interrupts
   * (keystroke order matters)
   */
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
 *  @param scancode The scancode to write into the buffer
 *
 *  @return Void.
 **/
static void __buffer_write(kh_type k)
{
  disable_interrupts();

  // Write into the buffer
  buff.buffer[buff.w] = k;
  buff.w = MODINC(buff.w);

  // Increment read as needed
  if (buff.count >= KBD_BUFFER_SIZE)
    buff.r = MODINC(buff.r);

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
 *  @param scancode Destination pointer for read scancode
 *
 *  @return The number of lost entries since the last read, or -1 if the
 *          keyboard buffer is currently empty.
 **/
static int __buffer_read(kh_type *kp)
{
  disable_interrupts();

  /* Check for an empty buffer */
  if (buff.count == 0) {
    enable_interrupts();
    return -1;
  }

  /* Read from the buffer */
  *kp = buff.buffer[buff.r];
  buff.r = MODINC(buff.r);

  --buff.count;
  enable_interrupts();
  return buff.count;
}

