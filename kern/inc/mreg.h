/** @file mreg.h
 *
 *  @brief Delcares the memory region API.
 *
 *  @author Enrique Naudon (esn)
 *  @author Marlies Ruck (mruck)
 **/
#ifndef __MREG_H__
#define __MREG_H__

#include <cllist.h>
#include <page_alloc.h>


/** @struct mem_region
 *  @brief A contiguous region in memory.
 **/
struct mem_region {
  void *start;          /**< The first byte in the region **/
  void *limit;          /**< The last byte in the region **/
  unsigned int attrs;   /**< Attributes for the region **/
  cll_node node;
};
typedef struct mem_region mem_region_s;


void mreg_init(mem_region_s *mreg, void *start, void *limit, unsigned int attrs);
int mreg_insert(cll_list *map, mem_region_s *new);
mem_region_s *mreg_lookup(cll_list *map, mem_region_s *targ);
mem_region_s *mreg_prev(cll_list *map, mem_region_s *targ);
mem_region_s *mreg_next(cll_list *map, mem_region_s *targ);
mem_region_s *mreg_extract(cll_list *map, mem_region_s *targ);


#endif /* __MREG_H__ */

