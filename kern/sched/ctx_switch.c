#include <ctx_switch.h>
#include <process.h>

#include <x86/eflags.h>
#define EFL_USER_MODE (EFL_RESV1 | EFL_IOPL_RING0 | EFL_IF) & ~EFL_AC

#include <simics.h>

void asm_ctx_switch(void *my_sp, void *my_pc, void *new_sp, void *new_pc,
                    unsigned int new_cr3, unsigned int kstack_high);
/* All args are in registers to preserve the stack */
void store_and_switch(void);

int i = 0;

void ctx_switch(void)
{
  /* Set the flags for the first ctx switch because we haven't launched this
   * task yet */
  if(i%2 == 0){
    i++;
    curr_pcb = &pcb2;
    asm_ctx_switch(&pcb1.my_tcb.sp, &pcb1.my_tcb.pc, pcb2.my_tcb.sp, 
                    pcb2.my_tcb.pc, pcb2.cr3, (unsigned int)
                    (&pcb2.my_tcb.kstack[KSTACK_SIZE -1]));
  }
  else{
    i++;
    curr_pcb = &pcb1;
    asm_ctx_switch(&pcb2.my_tcb.sp, &pcb2.my_tcb.pc, pcb1.my_tcb.sp, 
                    pcb1.my_tcb.pc, pcb1.cr3, (unsigned int)
                    (&pcb1.my_tcb.kstack[KSTACK_SIZE -1]));
  }
  return;
}
