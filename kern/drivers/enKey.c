/** @file keyboard.c 
 *
 *  @brief Contains a basic keyboard driver.
 *
 *  Upon each keyboard interrupt, we read scancodes from the keyboard port
 *  and store them into a scancode buffer.  Clients of our API can receive
 *  characters through readchar(...), which reads and processes the
 *  buffer's scancodes, to return the users' character presses.
 *
 *  Our scancode buffer is implemented as a basic circular buffer.  We
 *  maintain read and write indicies, at which we read and write characters,
 *  respectively.  When the buffer becomes full, subsiquent writes
 *  overwrite the oldest data in the buffer.
 *
 *  @author Enrique Naudon (esn)
 *
 *  @bugs No known bugs.
 **/

#include <interrupt_defines.h>
#include <keyhelp.h>
#include <x86/asm.h>

#include "keyboard_internal.h"


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
int readchar(void)
{
  int res;
  char scancode;
  kh_type k;

  while ((res = __buffer_read(&scancode)) != -1)
  {
    k = process_scancode(scancode);
    if (KH_HASDATA(k) && KH_ISMAKE(k))
      return KH_GETCHAR(k);
  }

  return -1;
}

/** @brief Handles keyboard interrupts.
 *
 *  Simply read a scancode from the keyboard and write it into the keyboard
 *  buffer.  All post-processing is saved for later (i.e. not in the
 *  context of an interrupt).
 *
 *  @return Void.
 **/
void kbd_interrupt_handler(void)
{
  char scancode;

  scancode = inb(KEYBOARD_PORT);
  __buffer_write(scancode);

  outb(INT_CTL_PORT, INT_ACK_CURRENT);
  return;
}


/*************************************************************************/
/* Internal helper functions                                             */
/*************************************************************************/

/* The keyboard buffer itself */
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
static void __buffer_write(char scancode)
{
  // Write into the buffer
  buff.buffer[buff.w] = scancode;
  buff.w = MODINC(buff.w);

  // Increment read as needed
  if (buff.count >= KBD_BUFFER_SIZE)
    buff.r = MODINC(buff.r);

  ++buff.count;
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
static int __buffer_read(char *scancode)
{
  int lost;
  
  disable_interrupts();

  /* Check for an empty buffer */
  if (buff.count == 0) {
    enable_interrupts();
    return -1;
  }

  /* Save lost and reset count */
  if (buff.count > KBD_BUFFER_SIZE) {
    lost = buff.count - KBD_BUFFER_SIZE;
    buff.count = KBD_BUFFER_SIZE;
  } else lost = 0;

  /* Read from the buffer */
  *scancode = buff.buffer[buff.r];
  buff.r = MODINC(buff.r);

  --buff.count;
  enable_interrupts();
  return lost;
}


/*************************************************************************/
/* Debugg Functions                                                      */
/*************************************************************************/

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
void write_scancode(char scancode)
{
  /* Disable interrupts to ensure the keyboard callback isn't writing into
   * the buffer at the same time.
   */
  disable_interrupts();
  __buffer_write(scancode);
  enable_interrupts();
  return;
}

