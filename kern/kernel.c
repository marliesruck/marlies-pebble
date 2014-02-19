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
  MAGIC_BREAK;

  pde_t pde = get_pde(pd, NULL);
  lprintf("NULL's PDE: 0x%08X", pde);
  pte_t pte = get_pte(pd, NULL);
  lprintf("NULL's PTE: 0x%08X", pte);

  lprintf("pd = %p", pd);
  MAGIC_BREAK;
  
  pde = get_pde(pd, pd);
  lprintf("pd's PDE: 0x%08X", pde);
  pte = get_pte(pd, pd);
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

  lprintf("get_pde(0) = 0x%08X", get_pde(pg_directory, &pg_tables[0]));
  lprintf("get_pde(1) = 0x%08X", get_pde(pg_directory, &pg_tables[1]));
  lprintf("get_pde(2) = 0x%08X", get_pde(pg_directory, &pg_tables[2]));
  lprintf("get_pde(3) = 0x%08X", get_pde(pg_directory, &pg_tables[3]));
  lprintf("get_pde(4) = 0x%08X", get_pde(pg_directory, &pg_tables[4]));
  lprintf("get_pde(5) = 0x%08X", get_pde(pg_directory, &pg_tables[5]));
  lprintf("get_pde(6) = 0x%X", get_pde(pg_directory, pg_tables[6]));
  
  lprintf( "Hello from a brand new kernel!" );

  while (1) {
      continue;
  }

  return 0;
}

