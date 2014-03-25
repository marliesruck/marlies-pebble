/** @file page_util.h
 *
 *  @brief Defines x86 specific functions for paging and protections.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

/* x86 specific includes */
#include <x86/cr.h>

/* Pebble specific includes */
#include <page_util.h>

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
