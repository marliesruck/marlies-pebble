/* @file task.c
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

/* Libc includes */
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>

/* Keep track of tasks */
static cll_list task_list = CLL_LIST_INITIALIZER(task_list);
/* And make all accesses atomic */
mutex_s task_list_lock = MUTEX_INITIALIZER(task_list_lock);

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

/** @brief Search the task list.
 *
 *  Grabs list lock, and assumes caller will release it.
 *
 *  @param task Address of task to search for.
 *
 *  @return A pointer to the task, or NULL if not found.
 **/
task_t *tasklist_find(task_t *t)
{
  assert(t);

  cll_node *n;
  task_t *task;

  /* Iteratively search the task list */
  mutex_lock(&task_list_lock);
  cll_foreach(&task_list, n) {
    task = cll_entry(task_t *,n);
    if (t == task) return task;
  }
  /* Does NOT release task lock */

  /* We didn't find it */
  return NULL;
}

