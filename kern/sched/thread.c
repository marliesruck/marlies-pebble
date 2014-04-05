/* @file thread.c
 * 
 * @brief Defines structures and functions for thread manipulation.
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 *
 * @bug List locking mechanisms? 
 *      Should vm initialization be here?
 *      should thread init be responsible for acquiring the tid lock?
 *      extern thrlist and initialize here
 *
 */
#include <simics.h>

/* Pebble specific includes */
#include <cllist.h>
#include <pg_table.h>
#include <process.h>
#include <sched.h>
#include <sc_utils.h>
#include <thread.h>
#include <vm.h>

/* Libc specific includes */
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>

/* Atomically acquire a tid */
static int tid = 0;
static mutex_s tid_lock = MUTEX_INITIALIZER(tid_lock); 

/** @var thread_list 
 *  @brief The list of threads.
 **/
static cll_list thread_list = CLL_LIST_INITIALIZER(thread_list);
static mutex_s thrlist_lock = MUTEX_INITIALIZER(thrlist_lock);


/* @brief Initialize a thread.
 *
 * Allocate a thread_t struct and atomically acquires a TID.
 *
 * @param task Task that thread belongs to.
 * @return Addres of initialized thread, or NULL if our of memory.
 */
thread_t *thread_init(task_t *task)
{
  assert(task);

  /* Allocate the thread structure */
  thread_t *thread = malloc(sizeof(thread_t));
  if(!thread) return NULL;

  /* Atomically acquire TID */
  mutex_lock(&tid_lock);
  thread->tid = ++tid;
  mutex_unlock(&tid_lock);

  /* Initialize other fields */
  mutex_init(&thread->lock);
  thread->state = THR_NASCENT;
  thread->task_info = task;
  thread->killed = 0;
  thread->sp = NULL;
  thread->pc = NULL;

  /* Embedded list traversal */
  cll_init_node(&thread->node, thread);
  cll_init_node(&thread->zzz_node, thread);
  cll_init_node(&thread->task_node, thread);
  cll_init_node(&thread->thrlist_entry, thread);

  /* No software exception handlers should be registered */
  deregister(&thread->swexn); 
    
  return thread;
}

int thr_launch(thread_t *t, void *sp, void *pc)
{

  /* Set stack and instruction pointers */
  t->sp = sp;
  t->pc = pc;

  /* Add the thread to the thread list */
  assert(!thrlist_add(t));

  /* Unblock (also sets state to THR_RUNNABLE */
  assert(!sched_unblock(t));

  return 0;
}


/*************************************************************************
 *  Thread List Functions
 *************************************************************************/

/** @brief Add a thread to the thread list.
 *
 *  @param t The thread to add.
 *
 *  @return -1 on error, else 0.
 **/
int thrlist_add(thread_t *t)
{
  /* Lock, insert, unlock */
  mutex_lock(&thrlist_lock);
  cll_insert(thread_list.next, &t->thrlist_entry);
  mutex_unlock(&thrlist_lock);

  return 0;
}

/** @brief Remove a thread from the thread list.
 *
 *  @param t The thread to delete.
 *
 *  @return -1 if thread not found, else 0.
 **/
int thrlist_del(thread_t *t)
{
  /* Lock, extract, unlock */
  mutex_lock(&thrlist_lock);
  assert(cll_extract(&thread_list, &t->thrlist_entry));
  mutex_unlock(&thrlist_lock);
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

  t = NULL;

  /* Iteratively search the thread list */
  mutex_lock(&thrlist_lock);
  cll_foreach(&thread_list, n) {
    t = cll_entry(thread_t *,n);
    if (t->tid == tid) break;
  }
  mutex_unlock(&thrlist_lock);

  /* We didn't find it */
  if (n == &thread_list) return NULL;

  return t;
}

