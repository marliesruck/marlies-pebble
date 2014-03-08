/* @file thread.c
 * 
 *
 * @bug List locking mechanisms? 
 *      queue vs list?
 */
#include <simics.h>

#include <thread.h>
#include <process.h>
#include <queue.h>

/* Atomically acquire a tid */
#include <spin.h>

/* For task vm initializiation, shoudl this be here? */
#include <vm.h>
#include <pg_table.h>

#include <stdlib.h>
#include <malloc.h>

static int tid = 0;

thread_t *task_init(void)
{
  task_t *task = malloc(sizeof(task_t));

  /* Initialize vm? */
  task->vmi = (vm_info_s) {
    .pg_info = (pg_info_s) {
      .pg_dir = (pte_s *)(TBL_HIGH),
      .pg_tbls = (pt_t *)(DIR_HIGH),
    },
    .mmap = CLL_LIST_INITIALIZER(task->vmi.mmap)
  };

  task->num_threads = 1;
  task->exited = 0;

  /* Initialize root thread */
  thread_t *thread = thread_init(task);

  task->orig_tid = thread->tid;

  return thread;
}

/* Atomically increment tid */
thread_t *thread_init(task_t *task)
{
  thread_t *thread = malloc(sizeof(thread_t));
  thread->task_info = task;
  /* ATOMICALLY */
  thread->tid = ++tid;
  thread->sp = NULL;
  thread->pc = NULL;
  
  return thread;
}

void thrlist_init(queue_s *q)
{
  queue_init(q);
  return;
}

void thrlist_enqueue(thread_t *thread, queue_s *q)
{
  queue_node_s *n = malloc(sizeof(queue_node_s));
  queue_init_node(n, thread);

  queue_enqueue(q, n);

  return;
}
thread_t *thrlist_dequeue(queue_s *q)
{
  queue_node_s *n = queue_dequeue(q);
  thread_t *thread = queue_entry(thread_t *, n);

  free(n);

  return thread;
}
