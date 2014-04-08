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
  /* Allocate a PCB */
  task_t *task = malloc(sizeof(task_t));
  if(!task) return NULL;

  /* Allocate a cll node */
  task->tasklist_entry = malloc(sizeof(cll_node));
  if (!task->tasklist_entry) {
    free(task);
    return NULL;
  }
  cll_init_node(task->tasklist_entry, task);

  /* Initialize vm */
  vm_init(&task->vmi);
  task->cr3 = (uint32_t) task->vmi.pg_info.pg_dir;

  /* Keep track of threads in a task */
  task->num_threads = 0;

  /* Keep track of children alive and dead */
  task->live_children = 0;
  queue_init(&task->dead_children);
  cvar_init((&task->cv));
  task->dead_thr = NULL;

  task->parent_tid = curr_tsk->tid;

  /* Initialize the task struct lock */
  mutex_init(&task->lock);

  /* Zero the status */
  task->status = 0;

  /* Initialize root thread */
  thread_t *thread = thread_init(task);
  if(!thread) {
    free(task->tasklist_entry);
    free(task);
    return NULL;
  }

  /* Add it to the new task */
  task_add_thread(task, thread);
  task->tid = thread->tid;

  /* Add to task list */
  tasklist_add(task);

  return thread;
}

void task_add_thread(task_t *tsk, thread_t *thr)
{
  mutex_lock(&tsk->lock);
  tsk->num_threads += 1;
  mutex_unlock(&tsk->lock);

  return;
}

void task_del_thread(task_t *tsk, thread_t *thr)
{
  /* Free the last dead thread */
  if (tsk->dead_thr)
    thr_free(tsk->dead_thr);

  /* Make yourself dead and dec thread count */
  tsk->dead_thr = thr;
  tsk->num_threads -= 1;

  return;
}

/** @brief Add a task to the task list.
 *
 *  @param t The task to add.
 *
 *  @return -1 on error, else 0.
 **/
void tasklist_add(task_t *t)
{
  assert(t);

  /* Lock, insert, unlock */
  mutex_lock(&task_list_lock);
  cll_insert(task_list.next, t->tasklist_entry);
  mutex_unlock(&task_list_lock);

  return;
}

/** @brief Remove a task from the task list.
 *
 *  @param t The task to delete.
 *
 *  @return -1 on error, else 0.
 **/
void tasklist_del(task_t *t)
{
  assert(t);

  /* Lock, delete, unlock */
  mutex_lock(&task_list_lock);
  assert(cll_extract(&task_list, t->tasklist_entry));
  mutex_unlock(&task_list_lock);
  return;
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
  while(!queue_empty(&task->dead_children)){
    n = queue_dequeue(&task->dead_children);
    victim = queue_entry(task_t *, n);
    task_reap(victim);
    free(n);
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

void task_enqueue_status(task_t *parent, task_t *child)
{
  /* Add your status */
  queue_enqueue(&parent->dead_children, child->tasklist_entry);

  /* Signal your parent */
  cvar_signal(&parent->cv);

  return;
}

void task_del_child(task_t *parent, task_t *child)
{
//  /* Free the most recently deceased task */
//  if (parent->dead_task)
//    task_reap(parent->dead_task);
//  parent->dead_task = child;

  /* Live children count is for nuclear family only */
  if(parent->tid == child->parent_tid)
    parent->live_children--;

  return;
}

/** @brief Wake your parent and remove yourself from the runnable queue.
 *
 *  @param task Task whose parent we should awaken.
 *
 *  @return Void.
 **/
task_t *task_signal_parent(task_t *task)
{
  task_t *parent = task_find_and_lock_parent(task);

  task_del_child(parent, task);

  task_enqueue_status(parent, task);

  return parent;
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
  free(n);
 
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
  /* Free the last thread to die */
  if (victim->dead_thr) thr_free(victim->dead_thr);

  /* Free the page directory and PCB */
  sfree((void *)(victim->cr3), PAGE_SIZE);
  free(victim);

  return;
}
 
