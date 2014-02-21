/** @file loader.c
 *
 *  @brief Load executable into memory
 *
 *  Takes advantage of the utility 410/exec20bj which takes a list of file names
 *  of Pebbles executables and builds a single file, user_apps.S, containing one
 *  large character array per executable.  This loader relies on
 *  410kern/elf/load_helper.c for elf_check_header() to verify the ELF header
 *  and elf_load_helper() for populate a "simplied elf" struct.  We then
 *  allocate frames, copy the segments, and map them to the requested virtual
 *  addresses in the user's page directory.
 *
 *  @author Marlies Ruck (mruck)
 *  @author Enrique Naudon (esn)
 *
 *  @bug Currently one entry in the page directory is self-referntial, what if
 *  the user expects an executeable to use that virtual address?
 *
 */
/*@{*/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <loader.h>
#include <elf_410.h>

#include <pg_table.h>
#include <frame_alloc.h>
#include <vm.h>

/* --- Debugging --- */
#include <simics.h>
#include <assert.h>

/* --- Local function prototypes --- */ 


/** @brief Copies data from a file into a buffer.
 *
 *  @param filename   the name of the file to copy data from
 *  @param offset     the location in the file to begin copying from
 *  @param size       the number of bytes to be copied
 *  @param buf        the buffer to copy the data into
 *
 *  @return returns the number of bytes copied on succes; -1 on failure
 */
int getbytes( const char *filename, int offset, int size, char *buf )
{
  int i;

  /* Search table of contents for file */
  for(i = 0; i < exec2obj_userapp_count; i++){
    if (0 == strcmp(filename,exec2obj_userapp_TOC[i].execname))
      break;
  }

  if (i >= exec2obj_userapp_count)
    assert(0);

  void *src = (void *)((unsigned)(exec2obj_userapp_TOC[i].execbytes) 
              + (unsigned)(offset));

  /* Copy size bytes from file starting at offset */
  memcpy(buf, src, size);

  return size;
}

/** @brief Loads file into memory 
 *
 *  Assumes page directory has already been initialized with 4 entries for
 *  kernel memory and one self-referential entry
 *
 *  @param filename File to be loaded
 */
void load_file(const char* filename)
{
  /* Validate elf header */
  if(0 > elf_check_header(filename))
    assert(0);

  /* Populate elf struct */
  simple_elf_t se;
  if(0 > elf_load_helper(&se,filename))
    assert(0);

  /* @bug Call vm_alloc with RO flags */
  load_segment(filename, se.e_txtoff,se.e_txtlen, se.e_txtstart, 
              PG_TBL_PRESENT);

  /* Zero out areas between .txt and .rodata */

  unsigned long text_end = se.e_txtstart + se.e_txtlen;
  memset((void *)(text_end),0,se.e_rodatstart - text_end);

  /* @bug Call vm_alloc with RO flags */
  load_segment(filename, se.e_rodatoff,se.e_rodatlen, se.e_rodatstart,
               PG_TBL_PRESENT);

  /* Zero out areas between .rodata and .data */
  unsigned long rodat_end = (unsigned)(se.e_rodatstart) + se.e_rodatlen;
  memset((void *)(rodat_end),0,(unsigned long)(se.e_datstart) - rodat_end);

  load_segment(filename, se.e_datoff,se.e_datlen, se.e_datstart, 
              PG_TBL_PRESENT | PG_TBL_WRITABLE | PG_TBL_USER );

  /* Zero out areas between .data and .bss */
  unsigned long dat_end = (unsigned)(se.e_datstart) + se.e_datlen;
  memset((void *)(dat_end),0,(unsigned long)(se.e_bssstart) - dat_end);

  /* Init bss */
  vm_alloc(pg_dir,(void*)(se.e_bssstart),se.e_bsslen,
           PG_TBL_PRESENT | PG_TBL_WRITABLE | PG_TBL_USER );
  memset((void *)(se.e_bssstart),0,se.e_bsslen);

  return;
}
/** @brief load_segment Loads single segment into memory
 *
 *  @param filename Name of file with segment
 *  @param offset Offset of segment in file
 *  @param len Length of segment in bytes
 *  @param start Start of segment virtual address
 */
void load_segment(const char* filename, unsigned long offset, 
                  unsigned long len, unsigned long start, unsigned int flags)
{
  /* Allocate frame(s) for section and map to virtual pages */
  vm_alloc(pg_dir,(void*)(start),len,flags);

  /* Read segment from the 'image' */
  char segment[len];
  getbytes(filename,offset,len,segment);

  /* Copy segment to virtual page(s) */
  memcpy((void*)(start),segment,len);

  return;
}

/*@}*/
