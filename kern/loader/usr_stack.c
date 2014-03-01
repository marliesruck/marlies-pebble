#include <usr_stack.h>
#include <x86/cr.h>
#include <process.h>

void *usr_stack_init(vm_info_s *vmi)
{
  /* Allocate user's stack */
  void *base = USR_SP_HI - USR_STACK_SIZE;
  vm_alloc(vmi, base, USR_STACK_SIZE, VM_ATTR_RDWR|VM_ATTR_USER);

  /* Initialize user's stack for _main() */
  void *sp = USR_SP_HI;
  PUSH(sp,base);            /* stack_low */
  PUSH(sp,USR_SP_HI);       /* stack_high */
  PUSH(sp,0);               /* argv */
  PUSH(sp,0);               /* argc */
  PUSH(sp,0);               /* "return address" */

//  set_esp0((uint32_t)(&my_pcb.my_tcb.kstack[KSTACK_SIZE - 1]));

  return sp;
}

