/* @file usr_stack.h
 *
 * @brief User stack initialization routine
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug USR_SP_HI should be moved down one tome and the tome its currently using
 * should be reserved for the child PDE
 */
#ifndef _USR_STACK_H
#define _USR_STACK_H

#include <pg_table.h>
#include <vm.h>
#include <x86/page.h>


#define USR_STACK_SIZE ( PAGE_SIZE * PG_TBL_ENTRIES )
#define USR_SP_HI ( (void *)tomes[PG_TBL_ENTRIES - 2] )

void *usr_stack_init(vm_info_s *vmi, char **argv);

#endif /* _USR_STACK_H */

