/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *
 *  @author Harry Q. Bovik (hqbovik)
 *  @author Fred Hacker (fhacker)
 *  @bug No known bugs.
 */

#include <common_kern.h>

/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */
#include <x86/cr.h>

#include <pg_table.h>
#include <frame_alloc.h>

/*************************************************************************
 *  Random paging stuff that should really be elsewhere
 *************************************************************************/

#include <x86/cr.h>
#define CR0_PAGE 0x80000000

void enable_paging(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 |= CR0_PAGE;
  set_cr0(cr0);

  return;
}

void disable_paging(void)
{
  uint32_t cr0;
  
  cr0 = get_cr0();
  cr0 &= ~CR0_PAGE;
  set_cr0(cr0);

  return;
}


/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
  /*
   * When kernel_main() begins, interrupts are DISABLED.
   * You should delete this comment, and enable them --
   * when you are ready.
   */

  /* Set up kernel PT and a PD */
  init_kern_pt();
  pde_t *pd = alloc_frame();
  lprintf("pd = %p", pd);
  init_pd(pd);
  set_cr3((uint32_t) pd);
  enable_paging();

  /* Just some testing stuff */
  lprintf("Start testing...");

  pde_t pde = get_pde(pd, NULL);
  lprintf("NULL's PDE: 0x%08X", pde);
  pte_t pte;
  get_pte(pd, NULL, &pte);
  lprintf("NULL's PTE: 0x%08X", pte);

  pde = get_pde(pd, pd);
  lprintf("pd's PDE: 0x%08X", pde);
  get_pte(pd, pd, &pte);
  lprintf("pd's PTE: 0x%08X", pte);

  lprintf("current PTs = %p", pg_tables);
  lprintf("current PD = %p", pg_directory);
  lprintf("PD[0] = 0x%08X", pg_directory[0]);
  lprintf("PD[1] = 0x%08X", pg_directory[1]);
  lprintf("PD[2] = 0x%08X", pg_directory[2]);
  lprintf("PD[3] = 0x%08X", pg_directory[3]);
  lprintf("PD[4] = 0x%08X", pg_directory[4]);
  lprintf("PD[5] = 0x%08X", pg_directory[5]);
  lprintf("PD[6] = 0x%08X", pg_directory[6]);

  void *addr = NULL;
  lprintf("get_pde(%p) = 0x%08X", addr, get_pde(pg_directory, addr));
  addr = (void *)(1 * PAGE_SIZE * PG_TBL_ENTRIES);
  lprintf("get_pde(%p) = 0x%08X", addr, get_pde(pg_directory, addr));
  addr = (void *)(2 * PAGE_SIZE * PG_TBL_ENTRIES);
  lprintf("get_pde(%p) = 0x%08X", addr, get_pde(pg_directory, addr));
  addr = (void *)(3 * PAGE_SIZE * PG_TBL_ENTRIES);
  lprintf("get_pde(%p) = 0x%08X", addr, get_pde(pg_directory, addr));
  addr = (void *)(4 * PAGE_SIZE * PG_TBL_ENTRIES);
  lprintf("get_pde(%p) = 0x%08X", addr, get_pde(pg_directory, addr));
  addr = (void *)(5 * PAGE_SIZE * PG_TBL_ENTRIES);
  lprintf("get_pde(%p) = 0x%08X", addr, get_pde(pg_directory, addr));
  addr = (void *)(6 * PAGE_SIZE * PG_TBL_ENTRIES);
  lprintf("get_pde(%p) = 0x%08X", addr, get_pde(pg_directory, addr));
  
  addr = NULL;
  get_pte(pg_directory, addr, &pte);
  lprintf("get_pte(%p) = 0x%08X", addr, pte);
  addr = (void *)(1 * PAGE_SIZE * PG_TBL_ENTRIES);
  get_pte(pg_directory, addr, &pte);
  lprintf("get_pte(%p) = 0x%08X", addr, pte);
  addr = (void *)(2 * PAGE_SIZE * PG_TBL_ENTRIES);
  get_pte(pg_directory, addr, &pte);
  lprintf("get_pte(%p) = 0x%08X", addr, pte);
  addr = (void *)(3 * PAGE_SIZE * PG_TBL_ENTRIES);
  get_pte(pg_directory, addr, &pte);
  lprintf("get_pte(%p) = 0x%08X", addr, pte);
  addr = (void *)(4 * PAGE_SIZE * PG_TBL_ENTRIES);
  get_pte(pg_directory, addr, &pte);
  lprintf("get_pte(%p) = 0x%08X", addr, pte);
  addr = (void *)(5 * PAGE_SIZE * PG_TBL_ENTRIES);
  get_pte(pg_directory, addr, &pte);
  lprintf("get_pte(%p) = 0x%08X", addr, pte);
  addr = (void *)(6 * PAGE_SIZE * PG_TBL_ENTRIES);
  get_pte(pg_directory, addr, &pte);
  lprintf("get_pte(%p) = 0x%08X", addr, pte);
  
#include <vm.h>
  char *blah = (void *)0x01CFFFFF;
  size_t size = 0x2405;

  lprintf(" ");
  if (get_pte(pg_directory, blah, &pte))
    lprintf("ENTRY FOR %p NOT PRESNET!!", blah);
  else lprintf("get_pte(%p) = 0x%08X", blah, pte);
  lprintf(" ");

  lprintf("Allocating...");
  lprintf("  done! got %p", vm_alloc(pg_directory, blah, size,
                            PG_TBL_PRESENT|PG_TBL_WRITABLE|PG_TBL_USER));
  int i;
  for (i = 0; i < size; ++i) {
    blah[i] = i;
  }

  lprintf(" ");
  if (get_pte(pg_directory, blah, &pte))
    lprintf("ENTRY FOR %p NOT PRESNET!!", blah);
  else lprintf("get_pte(%p) = 0x%08X", blah, pte);
  lprintf(" ");
  lprintf("--------------------------------------------------");
  lprintf(" ");
  addr = (void *)USER_MEM_START;
  if (get_pte(pg_directory, addr, &pte))
    lprintf("ENTRY FOR %p NOT PRESNET!!", addr);
  else lprintf("get_pte(%p) = 0x%08X", addr, pte);
  lprintf(" ");

  lprintf("Allocating...");
  lprintf("  done! got %p", vm_alloc(pg_directory, addr, size,
                            PG_TBL_PRESENT|PG_TBL_WRITABLE|PG_TBL_USER));
  

  lprintf(" ");
  if (get_pte(pg_directory, addr, &pte))
    lprintf("ENTRY FOR %p NOT PRESNET!!", addr);
  else lprintf("get_pte(%p) = 0x%08X", addr, pte);
  lprintf(" ");

  lprintf( "Hello from a brand new kernel!" );

  while (1) {
      continue;
  }

  return 0;
}

