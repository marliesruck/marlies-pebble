/* @file process.c
 * 
 * @brief Defines structures and functions for task manipulation.
 *
 * @author Enrique Naudon (esn)
 * @author Marlies Ruck (mruck)
 */

/* Pebble specific includes */
#include <cllist.h>
#include <process.h>
#include <sched.h>
#include <thread.h>

/* Libc specific includes */
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>


/*************************************************************************
 *  Task manipulation
 *************************************************************************/

/* @brief Initialize a task and its root thread.
 *
 * @return Address of the initializaed root thread or NULL if
 * initialization fails.
 */
thread_t *task_init(void)
{
  /* Allocate a PCB */
  task_t *task = malloc(sizeof(task_t));
  if(!task) return NULL;

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
  task->dead_task = NULL;

  task->parent_tid = TASK_TID(curr_tsk);

  /* Initialize the task struct lock */
  mutex_init(&task->lock);

  /* Initialize root thread and add it to the task*/
  thread_t *thread = thread_init(task);
  if(!thread) {
    vm_final(&task->vmi);
    sfree((void *)(task->cr3), PAGE_SIZE);
    free(task);
    return NULL;
  }
  task_add_thread(task);

  /* Allocate and initialize the mini PCB */
  task->mini_pcb = malloc(sizeof(mini_pcb_s));
  if (!task->mini_pcb) {
    thr_free(thread);
    vm_final(&task->vmi);
    sfree((void *)(task->cr3), PAGE_SIZE);
    free(task);
    return NULL;
  }
  TASK_STATUS(task) = 0;
  TASK_TID(task) = thread->tid;
  cll_init_node(&TASK_LIST_ENTRY(task), task);

  /* Add to task list */
  tasklist_add(task);

  return thread;
}

/** @brief Add a thread to a task.
 *
 *  Thread's don't keep an explicit list of their threads, so we just
 *  atomically increment the thread count.
 *
 *  @param tsk The task whose count we're incrementing.
 *
 *  @return Void.
 **/
void task_add_thread(task_t *tsk)
{
  mutex_lock(&tsk->lock);
  tsk->num_threads += 1;
  mutex_unlock(&tsk->lock);

  return;
}

/** @brief Delete a thread from a task.
 *
 *  This function is meant to be called by threads as they vanish.  The
 *  vanishing thread is responsible for freeing the thread that vanished
 *  before it, and pointing the dead_thr pointer at themself so the next
 *  thread to vanish can free them.  The last thread in a task is freed by
 *  the parent task (see task_final(...) and task_free(...)).
 *
 *  @param tsk The task whose count thread we're deleting.
 *  @param thr The thread to delete.
 *
 *  @return Void.
 **/
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

/** @brief Add a child to a task.
 *
 *  As with threads, tasks do not maintain a list of their children, so we
 *  just need to increment the child count.
 *
 *  @param parent A pointer to the parent task.
 *
 *  @return Void.
 **/
void task_add_child(task_t *parent)
{
  mutex_lock(&parent->lock);
  parent->live_children++;
  mutex_unlock(&parent->lock);
  return;
}

/** @brief Delete a child from a task.
 *
 *  This function is analagous to the task_del_thread(...), but for child
 *  tasks... vanishing children are supposed to call this function.  The
 *  vanishing child is responsible for freeing the child that vanished
 *  before it, and pointing the dead_task pointer at itself so the next
 *  child to vanish can free them.  Each task is responsible for freeing
 *  it's last child before it exits, and when it reaps (see task_final(...)
 *  and task_free(...)).
 *
 *  Furthermore, the vanishing child is responsible for enqueueing it's
 *  mini PCB in the parent's dead children list and signaling any waiting
 *  threads in their parent task, so they can be reaped.
 *
 *  @param parent The task whose child we're deleting.
 *  @param child The child to delete.
 *
 *  @return Void.
 **/
void task_del_child(task_t *parent, task_t *child)
{
  mini_pcb_s *mini = child->mini_pcb;

  /* Free the most recently deceased task */
  if (parent->dead_task)
    task_final(parent->dead_task);
  parent->dead_task = child;

  /* Live children count is for nuclear family only */
  if(TASK_TID(parent) == child->parent_tid)
    parent->live_children--;

  /* Add your status */
  queue_init_node(&TASK_LIST_ENTRY(child), mini);
  queue_enqueue(&parent->dead_children, &TASK_LIST_ENTRY(child));

  /* Broadcast to any parents waiting on you */
  cvar_broadcast(&parent->cv);

  return;
}

/** @brief Free as much of the running task as possible.
 *
 *  This function frees only those parts of the task that can be freed
 *  while that task is still running/runnable.  It is meant to be called by
 *  tasks as they vanish.  The task_final(...) function, which should be
 *  called by the task's parent, is responsible for freeing the reset of
 *  the task.
 *
 *  @param task Task to free.
 *
 *  @return Void.
 **/
void task_free(task_t *task)
{
  cll_node *n;
  mini_pcb_s *mini;

  /* Remove yourself from the task list so that your children cannot add
   * themselves to your dead children list */
  tasklist_del(task);

  /* Free your virtual memory */
  vm_final(&task->vmi);

  /* Free your mutex */
  mutex_final(&task->lock);

  /* Free your kids' resources */
  while(!queue_empty(&task->dead_children)){
    n = queue_dequeue(&task->dead_children);
    mini = queue_entry(mini_pcb_s *, n);
    free(mini);
  }

  /* Free your last dead child */
  if (task->dead_task) {
    task_final(task->dead_task);
    task->dead_task = NULL;
  }

  return;
}

/** @brief Reap a child task.
 *
 *  In contrast to task_free(...), task_final(...) frees those parts of a
 *  task which the task cannot free itself.  This includes the task's page
 *  directory and the kernel stack.  This function should be called by the
 *  task's parent.
 *
 *  We call this function task_final(...) because we are freeing the very
 *  last bits of the task.
 *
 *  @param victim Task whose resources should be freed.
 *
 *  @return Void.
 **/
void task_final(task_t *victim)
{
  /* Your kid should have freed their kid */
  assert(victim->dead_task == NULL);

  /* There should always be a last thread */
  assert(victim->dead_thr);

  /* Free the task's resources */
  thr_free(victim->dead_thr);
  sfree((void *)(victim->cr3), PAGE_SIZE);
  free(victim);

  return;
}

/** @brief Attempt to reap the exit status from a task's dead children.
 *
 *  If the task has any live children, this function will block until they
 *  die, or until some other thread reaps the last child.
 *
 *  @param task The task whose children we're reaping.
 *  @param status A pointer to write the status into.
 *
 *  @return The TID of the reaped child, or -1 if there are no children, or
 *  if some other thread reaps the last child before you.
 **/
int task_reap(task_t *task, int *status)
{
  queue_node_s *n;
  mini_pcb_s *mini;
  int tid;

  /* Atomically check for dead children */
  mutex_lock(&task->lock);

  while (queue_empty(&task->dead_children))
  {
    /* Return error for no kids */
    if (task->live_children == 0) {
        mutex_unlock(&task->lock);
        return -1;
    }

    /* Wait for a death */
    else cvar_wait(&task->cv, &task->lock);
  }

  /* Save child info */
  n = queue_dequeue(&task->dead_children);
  mini = queue_entry(mini_pcb_s *, n);
  tid = mini->tid;
  *status = mini->status;

  /* Free any recently dead children */
  if (task->dead_task) {
    task_final(task->dead_task);
    task->dead_task = NULL;
  }
 
  /* Relinquish lock */
  mutex_unlock(&task->lock);

  free(mini);
  return tid;
}


/*************************************************************************
 *  Task list manipulation
 *************************************************************************/

/** @var task_list 
 *  @brief The list of tasks.
 **/
static cll_list task_list = CLL_LIST_INITIALIZER(task_list);

/** @var task_list_lock
 *  @brief Serializes access to the task list.
 **/
mutex_s task_list_lock = MUTEX_INITIALIZER(task_list_lock);

/** @brief Add a task to the task list.
 *
 *  @param t The task to add.
 *
 *  @return Void.
 **/
void tasklist_add(task_t *t)
{
  assert(t);

  /* Lock, insert, unlock */
  mutex_lock(&task_list_lock);
  cll_insert(task_list.next, &TASK_LIST_ENTRY(t));
  mutex_unlock(&task_list_lock);

  return;
}

/** @brief Remove a task from the task list.
 *
 *  @param t The task to delete.
 *
 *  @return Void.
 **/
void tasklist_del(task_t *t)
{
  assert(t);
  mutex_lock(&task_list_lock);

  assert(cll_extract(&task_list, &TASK_LIST_ENTRY(t)));

  /* Let anyone holding your lock go */
  mutex_lock(&t->lock);
  mutex_unlock(&t->lock);

  mutex_unlock(&task_list_lock);
  return;
}

/** @brief Find and lock your parent's PCB.
 *
 *  Search the task list for your original parent.  In order to keep your
 *  parent from exiting while you are using their PCB, we take the parent's
 *  PCB lock before releasing the task list lock.  If your (original)
 *  parent does not exist (i.e. already exited), your are init's child.
 *
 *  It is the responsibility of the caller to release their parent's lock.
 *
 *  @param task Task whose parent to search for.
 *
 *  @return The address of your parent's PCB, or init's PCB if your parent
 *  does not exist.
 **/
task_t *tasklist_find_and_lock_parent(task_t *task)
{
  cll_node *n;
  task_t *parent;
  int parent_tid = task->parent_tid;

  mutex_lock(&task_list_lock);

  /* Search the task list for your parent */
  cll_foreach(&task_list, n)
  {
    parent = cll_entry(task_t *,n);

    /* Grab your parent's lock before releasing the task list lock */
    if (TASK_TID(parent) == parent_tid)
    {
      mutex_lock(&parent->lock);
      mutex_unlock(&task_list_lock);
      return parent;
    }
  }

  /* Your parent is dead, so you're init's problem */
  mutex_lock(&init->lock);
  mutex_unlock(&task_list_lock);
  return init;
}

