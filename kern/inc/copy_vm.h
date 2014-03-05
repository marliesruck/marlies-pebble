/** @file copy_vm.h
 *
 *  @brief Delcares the page table copying API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __COPY_VM_H__
#define __COPY_VM_H__

#include <common_kern.h>
#include <x86/page.h>
#include <pg_table.h>

/* Extract most significant 20 bits for addr field of pte_s */
#define FRAME(x)                ((unsigned int)(x) >> PG_TBL_SHIFT)

/* --- Mechanism for striding through VM --- */
typedef char page_t[PAGE_SIZE];
typedef page_t tome_t[PG_TBL_ENTRIES];

/* --- PD/PT Copying --- */
void copy_pg(pt_t *pt, void *src, void *frame);
void *copy_pte(pte_s *src, pte_s *dest, int i);
void copy_pg_dir(pte_s *src, pt_t *src_pg_tbl, pte_s *dest, pt_t *dest_pg_tbl);

#endif /* __COPY_VM__ */

