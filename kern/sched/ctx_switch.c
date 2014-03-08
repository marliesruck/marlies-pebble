#include <ctx_switch.h>
#include <process.h>
#include <thread.h>

#include <simics.h>

void asm_ctx_switch(void *my_sp, void *my_pc, void *new_sp, void *new_pc,
                    unsigned int new_cr3, void *kstack_high);
void store_and_switch(void);

void ctx_switch(void)
{
  thread_t *prev = curr;
  thrlist_enqueue(prev, &naive_thrlist);
  curr = thrlist_dequeue(&naive_thrlist);

  asm_ctx_switch(&prev->sp, &prev->pc, curr->sp, 
                  curr->pc, curr->task_info->cr3, 
                  (&curr->kstack[KSTACK_SIZE -1]));
  return;
}
