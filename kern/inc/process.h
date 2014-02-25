/* @file process.h
 *
 */
#ifndef _PROCESS_H
#define _PROCESS_H

#define KSTACK_SIZE 4096

typedef struct tcb{
  int tid;
  char kstack[KSTACK_SIZE];
}tcb_t;

typedef struct pcb{
  unsigned int pg_dir;
  tcb_t my_tcb;  
}pcb_t;

pcb_t my_pcb;

#endif /* _PROCESS_H */
