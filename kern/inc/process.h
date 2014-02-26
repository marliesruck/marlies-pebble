/* @file process.h
 *
 */
#ifndef _PROCESS_H
#define _PROCESS_H

#define KSTACK_SIZE 4096

typedef struct tcb{
  int tid;
  void *sp;
  void *pc;
  char kstack[KSTACK_SIZE];
}tcb_t;

typedef struct pcb{
  unsigned int pg_dir;
  tcb_t my_tcb;  
}pcb_t;

pcb_t my_pcb;
pcb_t your_pcb;

#endif /* _PROCESS_H */
