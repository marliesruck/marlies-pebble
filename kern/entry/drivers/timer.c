/** @file timer.c 
 *
 *  @brief A (very) basic timer driver.
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug No known bugs
 */

/* Timer includes */
#include "timer.h"

/* Pebble includes */
#include <interrupt_defines.h>
#include <sched.h>
#include <timer_defines.h>
#include <x86/asm.h>
#include <x86/seg.h>


volatile unsigned int ticks = 0;


/*************************************************************************/
/* External Interface                                                    */
/*************************************************************************/

/** @brief Handles timer interrupts.
 *
 *  @return Void.
 **/
void tmr_int_handler(void)
{
  /* Ack first, ask questions later
   * (Also increase tick count...)
   */
  outb(INT_CTL_PORT, INT_ACK_CURRENT);
  ticks += 1;

  schedule();
  return;
}

/** @brief Initializes the timer driver.
 *
 *  @param rate The number of cycles per interrupt.
 *
 *  @return Void.
 **/
void tmr_init(unsigned short rate)
{
  outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
  outb(TIMER_PERIOD_IO_PORT, rate & 0xFF);
  outb(TIMER_PERIOD_IO_PORT, rate >> 8);

  return;
}

/** @brief Returns the number of ticks since boot.
 *
 *  It is important to note that the quantity/duration represented by a
 *  "tick" is entirely dependent on how the timer rate passed to
 *  tmr_init(...) when the timer driver was initialized.
 *
 *  @return The number of ticks since boot.
 **/
unsigned long tm_get_ticks()
{
  unsigned int t;
  disable_interrupts();
  t = ticks;
  enable_interrupts();
  return t;
}

