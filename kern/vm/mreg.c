/** @file mreg.c
 *
 *  @brief Implements out memory region stuff.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 *
 *  @bug For mreg_bounds, do I need to add/sub 1?
 *
 **/

/* Memory region includes */
#include <mreg.h>

/* Libc includes */
#include <assert.h>
#include <malloc.h>
#include <stddef.h>

/*************************************************************************
 *  Memory map and region manipulation
 *************************************************************************/

void mreg_init(mem_region_s *mreg, void *start, void *limit, unsigned int attrs)
{
  /* Intialize the mem region struct */
  mreg->start = start;
  mreg->limit = limit;
  mreg->attrs = attrs;
  cll_init_node(&mreg->node, mreg);

  return;
}

enum order { ORD_LT, ORD_EQ, ORD_GT };
typedef enum order ord_e;

ord_e mreg_compare(mem_region_s *lhs, mem_region_s *rhs)
{
  if (lhs->start < rhs->start && lhs->limit < rhs->start)
    return ORD_LT;
  else if (lhs->start > rhs->limit && lhs->limit > rhs->limit)
    return ORD_GT;

  return ORD_EQ;
}


/** @brief Identify a memory region WITHOUT extracting it.
 *
 *  @param map Memory map to search.
 *  @param targ Memory region start address to search for.
 *
 *  @return NULL if not found, else the memory region.
 **/
mem_region_s *mreg_lookup(cll_list *map, mem_region_s *targ)
{
  cll_node *n;
  mem_region_s *mreg;

  cll_foreach(map, n) {
    mreg = cll_entry(mem_region_s *, n);
    if (mreg_compare(mreg, targ) == ORD_EQ) {
      return mreg;
    }
  }

  return NULL;
}

/** @brief Ordered insertion into a sorted memory map.
 *
 *  Order is based on memory address. Lower memory address occur earlier in the
 *  list.
 *
 *  @param map Memory map to insert in.
 *  @param new Memory region to add.
 *
 *  @return -1 if out of memory, else 0.
 **/
int mreg_insert(cll_list *map, mem_region_s *new)
{
  cll_node *n;
  mem_region_s *mreg;
  ord_e ord;

  /* Ordered insertion by address into the mem map*/
  cll_foreach(map, n){
    mreg = cll_entry(mem_region_s *, n);
    ord = mreg_compare(new, mreg);
    if((ord == ORD_LT) || (ord == ORD_EQ))
      break;
  }

  cll_insert(n, &new->node);

  return 0;
}

/** @brief Identify the memory region above you in memory.
 *
 *  @param map Memory map to search.
 *  @param targ Memory region to search for.
 *
 *  @return NULL if targ is the highest region in memory, else the 
 *   memory region.
 **/
mem_region_s *mreg_next(cll_list *map, mem_region_s *targ)
{
  cll_node *n;

  n = targ->node.next;
  if(n == map)
    return NULL;
  else
    return cll_entry(mem_region_s *, n);
}

/** @brief Identify the memory region below you in memory.
 *
 *  @param map Memory map to search.
 *  @param targ Memory region to search for.
 *
 *  @return NULL if targ is the lowest region in memory, else the 
 *   memory region.
 **/
mem_region_s *mreg_prev(cll_list *map, mem_region_s *targ)
{
  cll_node *n;

  n = targ->node.prev;
  if(n == map)
    return NULL;
  else
    return cll_entry(mem_region_s *, n);
}

/** @brief Identify extract a specific memory region
 *
 *  @param map Memory map to search.
 *  @param targ Memory region to extract.
 *
 *  @return NULL if not found, else the memory region.
 **/
mem_region_s *mreg_extract(cll_list *map, mem_region_s *targ)
{
  cll_node *n;
  mem_region_s *mreg;

  cll_foreach(map, n) {
    mreg = cll_entry(mem_region_s *, n);
    if (mreg_compare(mreg, targ) == ORD_EQ) {
      cll_extract(map, n);
      return mreg;
    }
  }

  return NULL;
}

