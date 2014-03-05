#include <process.h>
#define CHILD_PDE (void *)(0xF0000000)
#define KSTACK_LOW_OFFSET(esp,base)    ((unsigned int)(esp) - (unsigned int)(base))
#define KSTACK_HIGH_OFFSET(esp,high)    ((unsigned int)(high) - (unsigned int)(esp))

void *mem_map_child(void);
void *init_child_tcb(void *child_cr3);

/* Asm protpotypes */
int finish_fork(void);
