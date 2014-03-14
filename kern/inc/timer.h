/** @file timer.h
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#ifndef __TIMER_H__
#define __TIMER_H__


#define TMR_DEFAULT_RATE (TIMER_RATE/100)

void tmr_init(unsigned short rate);
unsigned int tmr_get_ticks();


#endif /* __TIMER_H__ */

