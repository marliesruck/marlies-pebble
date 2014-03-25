/* @file process.c
 * 
 * @brief Defines structures and functions for task manipulation.
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 */

#include <simics.h>

/* Pebble specific includes */
#include <cllist.h>
#include <process.h>
#include <sched.h>
#include <thread.h>

/* Libc specific includes */
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>

/** @var task_list 
 *  @brief The list of tasks.
 **/
static cll_list task_list = CLL_LIST_INITIALIZER(task_list);
/* And make all accesses atomic */
mutex_s task_list_lock = MUTEX_INITIALIZER(task_list_lock);

/* @brief Initialize a task and its root thread.
 *
 * @return Address of the initializaed root thread or NULL if our of memory.
 */
thread_t *task_init(void)
{
  task_t *task = malloc(sizeof(task_t));

  /* Initialize vm */
  vm_init(&task->vmi, PG_TBL_ADDR[PG_SELFREF_INDEX], PG_TBL_ADDR);

  /* Keep track of threads in a task */
  task->num_threads = 1;
  cll_init_list(&task->peer_threads);

  /* Keep track of children alive and dead */
  task->live_children = 0;
  cll_init_list(&task->dead_children);
  cvar_init((&task->cv));

  task->parent = curr->task_info;

  /* Initialize the task struct lock */
  mutex_init(&task->lock);

  /* Initialize root thread with new task */
  thread_t *thread = thread_init(task);
  task->orig_tid = thread->tid;

  /* Add to task list */
  if(tasklist_add(task) < 0)
    return NULL;  /* Out of memory! */


  return thread;
}

/** @brief Add a task to the task list.
 *
 *  @param t The task to add.
 *
 *  @return -1 on error, else 0.
 **/
int tasklist_add(task_t *t)
{
  assert(t);

  cll_node *n;

  /* Allocate a node for the new task */
	n = malloc(sizeof(cll_node));
  if (!n) return -1;
  cll_init_node(n, t);

  /* Lock, insert, unlock */
  mutex_lock(&task_list_lock);
  cll_insert(task_list.next, n);
  mutex_unlock(&task_list_lock);

  return 0;
}

/** @brief Remove a task from the task list.
 *
 *  @param t The task to delete.
 *
 *  @return -1 on error, else 0.
 **/
int tasklist_del(task_t *t)
{
  assert(t);

  cll_node *n;
  task_t *task;

  /* Lock the task list */
  mutex_lock(&task_list_lock);

  /* Find our task in the task list */
  cll_foreach(&task_list, n)
    if (cll_entry(task_t *,n) == t) break;

  if (cll_entry(task_t *,n) != t){
    mutex_unlock(&task_list_lock);
    return -1;
  }

  /* Extract the task */
  assert(cll_extract(&task_list, n));
  task = cll_entry(task_t *, n);

  /* Unlock  */
  mutex_unlock(&task_list_lock);

  /* Free the node but NOT the task */
  free(n);

  return 0;
}

/** @brief Free a task's resources.
 *
 *  Removes task from global task list, frees its virtual memory and gives it
 *  dead children to init.
 *
 *  @param task Take to free.
 *
 *  @return Void.
 **/
void task_free(task_t *task)
{
  cll_node *n;

  /* Remove yourself from the task list so that your children cannot add
   * themselves to your dead children list */
  tasklist_del(task);

  /* Acquire and release your lock in case your child grabbed your task lock
   * before you removed yourself from the task list */
  mutex_lock(&task->lock);
  mutex_unlock(&task->lock);

  /* Free your virtual memory */
  vm_final(&task->vmi);

  /* Make your dead children the responsibility of init */
  while(!cll_empty(&task->dead_children)){
    n = cll_extract(&task->dead_children, task->dead_children.next);
    mutex_lock(&init->lock);
    cll_insert(&init->dead_children, n);
    mutex_unlock(&init->lock);
  }

  return;
}

/** @brief Wake your parent and remove yourself from the runnable queue.
 *
 *  @param task Task whose parent we should awaken.
 *
 *  @return Void.
 **/
void task_signal_parent(task_t *task)
{
  cll_node *n;
  task_t *t = NULL; 

  /* Check if your parent is still alive */
  task_t *parent = task->parent;

  mutex_lock(&task_list_lock);
  cll_foreach(&task_list, n) {
    t = cll_entry(task_t *,n);
    if (t == parent) break;
  }

  if(t != parent) 
    /* Your parent is dead. Tell init instead */
    parent = init; 
  else
  /* Live children count is for direct descendents only.  Don't do this if
   * your parent is init */
    parent->live_children--;

  /* Grab your parent's lock to prevent him/her from exiting while you are trying
   * to add yourself as a dead child */
  mutex_lock(&parent->lock);

  /* Release the list lock */
  mutex_unlock(&task_list_lock);

  /* Add yourself as a dead child */
  n = malloc(sizeof(cll_node));
  cll_init_node(n, task);
  cll_insert(&parent->dead_children, n);
  cvar_signal(&parent->cv);

  /* Your parent should not reap you until you've descheduled yourself */
  sched_mutex_unlock_and_block(curr, &parent->lock);

  return;
}
