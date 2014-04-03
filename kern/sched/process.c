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
task_t *task_find_and_lock_parent(task_t *task);

/* @brief Initialize a task and its root thread.
 *
 * @return Address of the initializaed root thread or NULL if our of memory.
 */
thread_t *task_init(void)
{
  task_t *task = malloc(sizeof(task_t));
  if(!task) return NULL;

  /* Initialize vm */
  vm_init(&task->vmi);
  task->cr3 = (uint32_t) task->vmi.pg_info.pg_dir;

  /* Keep track of threads in a task */
  task->num_threads = 0;
  cll_init_list(&task->peer_threads);

  /* Keep track of children alive and dead */
  task->live_children = 0;
  queue_init(&task->dead_children);
  cvar_init((&task->cv));

  task->parent_tid = curr->task_info->tid;

  /* Initialize the task struct lock */
  mutex_init(&task->lock);

  /* Initialize root thread */
  thread_t *thread = thread_init(task);
  if(!thread) {
    free(task);
    return NULL;
  }

  /* Add it to the new task */
  if (task_add_thread(task, thread)) {
    free(thread);
    free(task);
    return NULL;
  }
  task->tid = thread->tid;

  /* Add to task list */
  if(tasklist_add(task) < 0) return NULL;  

  return thread;
}

int task_add_thread(task_t *tsk, thread_t *thr)
{
  /* Allocate a node */
  cll_node *n = malloc(sizeof(cll_node));
  if(!n) return -1;

  /* Add the thread to the task */
  cll_init_node(n, thr);

  mutex_lock(&tsk->lock);
  cll_insert(&tsk->peer_threads, n);
  tsk->num_threads += 1;
  mutex_unlock(&tsk->lock);

  return 0;
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
  assert(tasklist_del(task) == 0);

  /* Acquire and release your lock in case your child grabbed your task lock
   * before you removed yourself from the task list */
  mutex_lock(&task->lock);
  mutex_unlock(&task->lock);

  /* Free your virtual memory */
  vm_final(&task->vmi);

  /* Free your kids' resources */
  while(!queue_empty(&task->dead_children)){
    n = queue_dequeue(&task->dead_children);
    victim = queue_entry(task_t *, n);
    task_reap(victim);
    /* Node is statically allocated so we don't free it */
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
 *  @param task Task whose parent to search for.
 *
 *  @return Address of init if your parent is dead, else address of your parent.
 **/
task_t *task_find_and_lock_parent(task_t *task)
{
  cll_node *n;
  task_t *t = NULL; 
  task_t *parent;
  int parent_tid = task->parent_tid;

  mutex_lock(&task_list_lock);
  cll_foreach(&task_list, n) {
    t = cll_entry(task_t *,n);
    if (t->tid == parent_tid) break;
  }

  /* Your parent is dead. Tell init instead */
  if(t->tid != parent_tid) 
    parent = init; 
  else
    parent = t;

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
  queue_node_s n;

  task_t *parent = task_find_and_lock_parent(task);

  /* Live children count is for nuclear family only */
  if(parent->tid == task->parent_tid)
    parent->live_children--;

  /* Add yourself as a dead child */
  queue_init_node(&n, task);
  queue_enqueue(&parent->dead_children, &n);

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
  queue_node_s *n;

  /* Atomically check for dead children */
  mutex_lock(&task->lock);

  while(queue_empty(&task->dead_children)){
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
  n = queue_dequeue(&task->dead_children);
  task_t *child_task = queue_entry(task_t*, n);
 
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
 *
 *  @return Void.
 **/
void task_reap(task_t *victim)
{
  cll_node *n;

  /* Free task's page directory */
  sfree((void *)(victim->cr3), PAGE_SIZE);

  /* Free task's thread resources */
  while(!cll_empty(&victim->peer_threads)){
    n = cll_extract(&victim->peer_threads, victim->peer_threads.next);
    free(n->data);
    free(n);
  }

  /* Free task struct */
  free(victim);
}
 
