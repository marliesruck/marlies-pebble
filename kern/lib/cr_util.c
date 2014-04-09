/** @file cr_util.h
 *
 *  @brief Defines x86 specific functions for control registers.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

/* x86 specific includes */
#include <x86/cr.h>
#include <eflags.h>

/* Pebble specific includes */
#include <cr_util.h>

/** @brief Determine whether or not interrupts are enabled 
 *
 *  @return 1 if enabled, else 0.
 **/
int interrupts_enabled(void)
{
  unsigned int flags = get_eflags();
  return (flags & EFL_IF);
}

void enable_write_protect(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 |= CR0_WP;
  set_cr0(cr0);

  return;
}

void enable_paging(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 |= CR0_PG;
  set_cr0(cr0);

  return;
}

void disable_paging(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 &= ~CR0_PG;
  set_cr0(cr0);

  return;
}
