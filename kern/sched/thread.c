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
 *      extern thrlist and initialize here
 *
 */

#include <assert.h>

#include <cllist.h>
#include <thread.h>
#include <process.h>
#include <queue.h>

#include <mutex.h>
#include <spin.h>

#include <vm.h>
#include <pg_table.h>

#include <stdlib.h>
#include <malloc.h>

mutex_s thrlist_lock = MUTEX_INITIALIZER(thrlist_lock);

/* Atomically acquire a tid */
static int tid = 0;
static spin_s tid_lock = SPIN_INITIALIZER();
static cll_list thread_list = CLL_LIST_INITIALIZER(thread_list);

/* @brief Initializae a task and its root thread.
 *
 * @return Address of the initializaed root thread.
 */
thread_t *task_init(void)
{
  task_t *task = malloc(sizeof(task_t));

  /* Initialize vm? */
  vm_init(&task->vmi, PG_TBL_ADDR[PG_SELFREF_INDEX], PG_TBL_ADDR);

  /* Keep track of the number of threads in a task */
  task->num_threads = 1;

  /* Keep track of children alive and dead */
  task->live_children = 0;
  queue_init(&task->dead_children);
  cvar_init((&task->cv));

  task->exited = 0;

  /* Initialize root thread with new task */
  thread_t *thread = thread_init(task);
  task->orig_tid = thread->tid;

  /* Initialize the task struct lock */
  mutex_init(&task->lock);



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
  thread->state = THR_NASCENT;

  mutex_init(&thread->lock);

  /* Atomically acquire TID */
  spin_lock(&tid_lock);
  thread->tid = ++tid;
  spin_unlock(&tid_lock);

  thread->sp = NULL;
  thread->pc = NULL;
    
    return thread;
}


/*************************************************************************
 *  Thread List Functions
 *************************************************************************/

/** @brief Add a thread to the thread lust.
 *
 *  @param t The thread to add.
 *
 *  @return Void.
 **/
int thrlist_add(thread_t *t)
{
  cll_node *n;

  /* Allocate a node for the new thread */
	n = malloc(sizeof(cll_node));
  if (!n) return -1;
  cll_init_node(n, t);

  cll_insert(thread_list.next, n);

  return 0;
}

/** @brief Remove a thread from the thread list.
 *
 *  @param t The thread to delete.
 *
 *  @return Void.
 **/
int thrlist_del(thread_t *t)
{
  cll_node *n;

  /* Find our thread in the thread list */
  cll_foreach(&thread_list, n)
    if (cll_entry(thread_t *,n) == t) break;
  if (cll_entry(thread_t *,n) != t)
    return -1;

  /* Extract and free it */
  assert(cll_extract(&thread_list, n));
  free(n);

  return 0;
}

/** @brief Search the thread list by TID for a specific thread.
 *
 *  @param tid The TID of the thread to look for.
 *
 *  @return A pointer to the thread, or NULL if not found.
 **/
thread_t *thrlist_find(int tid)
{
  cll_node *n;
  thread_t *t;

  /* Iteratively search the thread list */
  cll_foreach(&thread_list, n) {
    t = cll_entry(thread_t *,n);
    if (t->tid == tid) {
      return t;
    }
  }

  return NULL;
}

