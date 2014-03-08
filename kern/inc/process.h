/* @file process.h
 *
 */
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <vm.h>

#include <x86/page.h>
#include <stdint.h>


typedef struct task{
  uint32_t cr3;           /* PTBR */
  vm_info_s vmi;          /* Virtual Memory */
  uint32_t num_threads;   /* For knowing when vanish() should 
                             deallocate ALL resources */
  int exit_status;        /* Upon exiting write my status here and let my 
                             parent or init() reap my pcb */
  int exited;             /* Necessary in case exit status is 0 */
  int orig_tid;           /* Wait() returns the TID of the origin thread of the 
                             exiting tasks, not the tid of the last thread 
                             to vanish */
}task_t;

#endif /* _PROCESS_H */
