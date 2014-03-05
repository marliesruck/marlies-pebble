/* @file process.h
 *
 */
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <vm.h>

#include <x86/page.h>
#include <stdint.h>


#define KSTACK_SIZE PAGE_SIZE

typedef struct tcb{
  int tid;
  void *sp;
  void *pc;
  char kstack[KSTACK_SIZE];
}tcb_t;

typedef struct pcb{
  uint32_t cr3;  
  vm_info_s vmi;
  tcb_t my_tcb;  
}pcb_t;

/* For exec debugging */
pcb_t pcb1;
pcb_t pcb2;

/* For fork debugging*/
pcb_t child_pcb;

/* There should also be a curr tcb so we only flush the tlb when task switching */
pcb_t *curr_pcb;

#endif /* _PROCESS_H */
