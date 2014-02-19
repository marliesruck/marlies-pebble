/** @file pg_table.c
 *
 *  @brief Implements page table-/directory-manipulating functions.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/

#include <pg_table.h>

#include <syscall.h>

/* --- Page directory functions --- */
void init_pde(pde_t *pd)
{
  return;
}

void insert_pde(pde_t *pd, void *addr, pte_t *pt, unsigned int flags)
{
  return;
}

/* --- Page table functions --- */
void init_pte(pte_t *pd)
{
  return;
}

void insert_pte(pde_t *pd, void *addr, void *frame, unsigned int flags)
{
  return;
}

