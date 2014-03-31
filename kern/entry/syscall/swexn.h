/** @file swexn.h
 *
 *  @brief Declares globals for calling swexn routine from fault handler.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (esn)
 **/

#ifndef __SWEXN_H__
#define __SWEXN_H__

/* Pebbles specfic includes */
#include <ureg.h>
#include <sc_utils.h>

void init_exn_stack(ureg_t *state, unsigned int cause, void *cr2);

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

