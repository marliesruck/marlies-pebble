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

/*** --- Internal helper routines --- ***/
task_t *task_find_and_lock_parent(task_t *parent);

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
 *  Removes task from global task list, frees its virtual memory, reaps its dead
 *  children and gives it dead children to init.  Note that all these are
 *  operations that can be done easily by the vanishing task.  The parent is
 *  responisble for freeing the vanishing task's kernel stacks, page directory
 *  and task struct in task_reap().
 *
 *  @param task Task to free.
 *
 *  @return Void.
 **/
void task_free(task_t *task)
{
  cll_node *n;
  task_t *victim;

  /* Remove yourself from the task list so that your children cannot add
   * themselves to your dead children list */
  tasklist_del(task);

  /* Acquire and release your lock in case your child grabbed your task lock
   * before you removed yourself from the task list */
  mutex_lock(&task->lock);
  mutex_unlock(&task->lock);

  /* Free your virtual memory */
  vm_final(&task->vmi);

  /* Free your kids' resources */
  while(!cll_empty(&task->dead_children)){
    n = cll_extract(&task->dead_children, task->dead_children.next);
    victim = cll_entry(task_t *, n);
    task_reap(victim, task);
  }

  return;
}
/** @brief Find your parent and lock their task struct
 *
 *  Atomically searches the task list for your parent's task struct in order to
 *  determine if your parent is still alive.  If so, locks your parent's task
 *  struct before releasing the task list lock.
 *
 *  Assumes the caller will release the parent's lock.
 *
 *  @param parent Address of your parent.
 *
 *  @return Address of init if your parent is dead, else address of your parent.
 **/
task_t *task_find_and_lock_parent(task_t *parent)
{
  cll_node *n;
  task_t *t = NULL; 

  mutex_lock(&task_list_lock);
  cll_foreach(&task_list, n) {
    t = cll_entry(task_t *,n);
    if (t == parent) break;
  }

  /* Your parent is dead. Tell init instead */
  if(t != parent) 
    parent = init; 

  /* Grab your parent's lock to prevent him/her from exiting while you are trying
   * to add yourself as a dead child */
  mutex_lock(&parent->lock);

  /* Release the list lock */
  mutex_unlock(&task_list_lock);

  return parent;
}

/** @brief Wake your parent and remove yourself from the runnable queue.
 *
 *  @param task Task whose parent we should awaken.
 *
 *  @return Void.
 **/
void task_signal_parent(task_t *task)
{
  cll_node n;

  task_t *parent = task_find_and_lock_parent(task->parent);

  /* Live children count is for nuclear family only */
  if(parent == task->parent)
    parent->live_children--;

  /* Add yourself as a dead child */
  cll_init_node(&n, task);
  cll_insert(&parent->dead_children, &n);

  /* Signal your parent */
  cvar_signal(&parent->cv);

  /* Your parent should not reap you until you've descheduled yourself */
  sched_mutex_unlock_and_block(curr, &parent->lock);

  return;
}
/** @brief Search for a zombie child.
 *
 *  @param task Task to search for.
 *
 *  @return NULL if you have no child, else the address of the zombie.
 **/
task_t *task_find_zombie(task_t *task)
{
  cll_node *n;

  /* Atomically check for dead children */
  mutex_lock(&task->lock);

  while(cll_empty(&task->dead_children)){
    if(task->live_children == 0){
        /* Avoid unbounded blocking */
        mutex_unlock(&task->lock);
        return NULL;
      }
    else{
      cvar_wait(&task->cv, &task->lock);
    }
  }

  /* Remove dead child */
  n = cll_extract(&task->dead_children, task->dead_children.next);
 
  task_t *child_task = cll_entry(task_t*, n);
 
  /* Relinquish lock */
  mutex_unlock(&task->lock);
 
  return child_task;
}
/** @brief Reap a child task.
 *
 *  The parent frees the child's page directory, kernel stacks and task struct.
 *  Note how this differs from task_free(): these are all resources that are
 *  difficult for the vanishing task itself to free.
 *
 *  @param victim Task whose resources should be freed.
 *  @param reaper Task responsible for reaping.
 **/
void task_reap(task_t *victim, task_t *reaper)
{
  /* Free task's page directory */
  free_unmapped_frame((void *)(victim->cr3), &reaper->vmi.pg_info);

  /* Free task's thread resources... */
  cll_free(&victim->peer_threads);

  /* Free task struct */
  free(victim);
}
 
