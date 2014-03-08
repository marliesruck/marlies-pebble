/* @file thread.c
 * 
 * @brief Defines structures and functions for thread manipulation.
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug List locking mechanisms? 
 *      queue vs list?
 *      Should vm initialization be here?
 *      should thread init be responsible for acquiring the tid lock?
 *
 */
#include <simics.h>
#include <assert.h>

#include <thread.h>
#include <process.h>
#include <queue.h>

/* Atomically acquire a tid */
#include <spin.h>

#include <vm.h>
#include <pg_table.h>

#include <stdlib.h>
#include <malloc.h>

static int tid = 0;

/* @brief Initializae a task and its root thread.
 *
 * @return Address of the initializaed root thread.
 */
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

  /* Keep track of the number of threads in a task */
  task->num_threads = 1;

  task->exited = 0;

  /* Initialize root thread with new task */
  thread_t *thread = thread_init(task);

  /* Keep track of the root thread's tid because wait() expected this as the ret
   * val */
  task->orig_tid = thread->tid;

  return thread;
}

/* @brief Initialize a thread.
 *
 * Allocate a thread_t struct and atomically acquires a TID.
 *
 * @param task Task that thread belongs to.
 * @return Addres of initialized thread.
 */
thread_t *thread_init(task_t *task)
{
  assert(task);

  thread_t *thread = malloc(sizeof(thread_t));
  thread->task_info = task;
  thread->tid = ++tid;

  thread->sp = NULL;
  thread->pc = NULL;
  
  return thread;
}

/* @brief Initialize a thread list.
 *
 * @param q Address of queue to be initialized.
 * @return Void.
 */
void thrlist_init(queue_s *q)
{
  assert(q);

  queue_init(q);
  return;
}

/* @brief Enqueue a thread.
 *
 * @param thread Thread to be enqueued.
 * @param q Queue to enqueue thread in.
 * @return Void. 
 */
void thrlist_enqueue(thread_t *thread, queue_s *q)
{
  assert(thread);
  assert(q);

  queue_node_s *n = malloc(sizeof(queue_node_s));
  queue_init_node(n, thread);

  queue_enqueue(q, n);

  return;
}

/* @brief Dequeue head thread
 *
 * @param q Queue to enqueue thread in.
 * @return Address of dequeued thread
 */
thread_t *thrlist_dequeue(queue_s *q)
{
  assert(q);
  queue_node_s *n = queue_dequeue(q);
  thread_t *thread = queue_entry(thread_t *, n);

  free(n);

  return thread;
}
