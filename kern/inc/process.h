/* @file process.h
 *
 */
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <vm.h>

#include <x86/page.h>


#define KSTACK_SIZE PAGE_SIZE

typedef struct tcb{
  int tid;
  char kstack[KSTACK_SIZE];
}tcb_t;

typedef struct pcb{
  unsigned int pg_dir;
  vm_info_s vmi;
  tcb_t my_tcb;  
}pcb_t;

pcb_t my_pcb;

#endif /* _PROCESS_H */
