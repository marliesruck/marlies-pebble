/** @file swexn.h
 *
 *  @brief Declares globals for calling swexn routine from fault handler.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#ifndef __SWEXN_H__
#define __SWEXN_H__

#include <ureg.h>

typedef void (*swexn_handler_t)(void *arg, ureg_t *ureg);

extern void *swexn_sp;
extern swexn_handler_t swexn_fun;
extern void *swexn_arg;

void init_exn_stack(ureg_t *state);

/** @brief Swap ESP to exception stack and jump to handler.
 *  
 *  @param eip Exception handler.
 *  @param esp3 Exception handler stack.
 *
 *  @return Void.
 */
void run_handler(swexn_handler_t eip, void *esp3);

/** @brief Adopt register values requested by user.
 *
 *  @param state Execution state to be adopted.
 *
 *  @return Void.
 **/
void craft_state(ureg_t state);

/** @brief Get EBP.
 *
 *  @return Value in EBP.
 **/
void *get_ebp(void);


#endif /* __SWEXN_H__ */

