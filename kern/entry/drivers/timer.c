/* @file timer.c
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug No known bugs
 */

/* Timer includes */
#include "timer.h"

/* Pebbles includes */
#include <interrupt_defines.h>
#include <sched.h>
#include <timer_defines.h>

/* Libc includes */
#include <x86/asm.h>

#define CYCLES  (TIMER_RATE/100) /* 10 ms resolution */
#define LSB(x)  (x & 0xFF)
#define MSB(x)  ((x & 0xFF00) >> 8)

void tick(unsigned int ticks)
{
  schedule();
  return;
}

void init_timer(void)
{
    outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
    outb(TIMER_PERIOD_IO_PORT,LSB(CYCLES));
    outb(TIMER_PERIOD_IO_PORT,MSB(CYCLES));
    return;
}

void int_timer(void)
{
  outb(INT_CTL_PORT,INT_ACK_CURRENT);
  tick(++numTicks);
  return;
}
