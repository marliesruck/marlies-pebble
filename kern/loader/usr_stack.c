/** @file usr_stack
 **/
#include <simics.h>

#include <usr_stack.h>

#include <vm.h>
#include <util.h>

#include <malloc.h>
#include <string.h>
#include <x86/cr.h>


void *usr_stack_init(vm_info_s *vmi, char **arg_vec)
{
  void *base, *sp;
  char **argv = NULL;
  int argc = 0;
  int i;

  /* Allocate user's stack */
  base = USR_SP_HI - USR_STACK_SIZE;
  vm_alloc(vmi, base, USR_STACK_SIZE, VM_ATTR_USER | VM_ATTR_RDWR);
  sp = USR_SP_HI;

  /* Copy the argument vector onto the stack */
  if (arg_vec)
  {
    /* Calculate argc and malloc argv */
    for (i = 0; arg_vec[i] != NULL; ++i) continue;
    argc = i;
    argv = malloc(i * sizeof(char *));
    if (!argv) return NULL;

    /* Copy each arg string onto the user stack */
    for (i = 0; i < argc; ++i)
    {
      sp = sp - CEILING(strlen(arg_vec[i]) + 1, sizeof(unsigned int));
      strcpy(sp, arg_vec[i]);
      argv[i] = sp;
    }

    /* Copy argv into the user stack */
    sp = sp - CEILING(i * sizeof(char *), sizeof(unsigned int));
    memcpy(sp, argv, i * sizeof(char *));

    /* Free up argv and point it to the stack */
    free(argv);
    argv = sp;
  }

  /* Push _main(...)'s arguments */
  PUSH(sp,base);            /* stack_low */
  PUSH(sp,USR_SP_HI);       /* stack_high */
  PUSH(sp,argv);            /* argv */
  PUSH(sp,argc);            /* argc */
  PUSH(sp,0);               /* "return address" */

  return sp;
}

