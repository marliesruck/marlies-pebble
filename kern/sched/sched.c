#include <sched.h>
#include <process.h>

void asm_ctx_switch(void *my_sp, void *my_pc, void *new_sp, void *new_pc,
                    unsigned int new_cr3);
/* All args are in registers to preserve the stack */
void store_and_switch(void);

void ctx_switch(void)
{
  asm_ctx_switch(&my_pcb.my_tcb.sp, &my_pcb.my_tcb.pc,
                 your_pcb.my_tcb.sp, your_pcb.my_tcb.pc, your_pcb.pg_dir);
  /*
  asm_ctx_switch(&my_pcb[0].my_tcb.sp, &my_pcb[0].my_tcb.pc,
                 my_pcb[1].my_tcb.sp, my_pcb[1].my_tcb.pc, my_pcb[1].pg_dir);
                 */
  return;
}
