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

/* --- Debugging --- */
#include <assert.h>
#include <simics.h>


/* --- Local function prototypes --- */ 
void load_segment(const char* filename, int offset, size_t len,
                  unsigned long start);

/** @brief Copies data from a file into a buffer.
 *
 *  @param filename The name of the file to copy data from.
 *  @param offset The location in the file to begin copying from.
 *  @param size The number of bytes to be copied.
 *  @param buf The buffer to copy the data into.
 *.
 *  @return The number of bytes copied on succes; -1 on failure.
 */
int getbytes( const char *filename, int offset, int size, char *buf )
{
  void *src;
  int i;

  /* Search table of contents for file */
  for(i = 0; i < exec2obj_userapp_count; i++){
    if (0 == strcmp(filename,exec2obj_userapp_TOC[i].execname))
      break;
  }

  /* Error if we didn't find the file */
  if (i >= exec2obj_userapp_count)
    assert(0);

  /* Copy size bytes from file starting at offset */
  src = (void *)&exec2obj_userapp_TOC[i].execbytes[offset];
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
void *load_file(vm_info_s *vmi, const char* filename)
{
  simple_elf_t se;
  void *ret;

  /* Validate header and populate elf struct */
  assert(elf_check_header(filename) == 0);
  assert(elf_load_helper(&se,filename) == 0);

  /* For simplicity, we assume text < rodata and data < bss */
  assert(se.e_txtstart < se.e_rodatstart);
  assert(se.e_datstart < se.e_bssstart);

  /* Allocate read/execute memory */
  lprintf("load_file(vmi = %p, filename = %s)", vmi, filename);
  ret = vm_alloc(vmi, (void *)se.e_txtstart,
                 (se.e_rodatstart - se.e_txtstart) + se.e_rodatlen,
                 PG_TBL_PRESENT | PG_TBL_USER);
  assert(ret != NULL);

  /* Load read/execute sections (text and rodata) */
  load_segment(filename, se.e_txtoff, se.e_txtlen, se.e_txtstart);
  memset((void *)(se.e_txtstart + se.e_txtlen), 0,
         se.e_rodatstart - (se.e_txtstart + se.e_txtlen));
  load_segment(filename, se.e_rodatoff, se.e_rodatlen, se.e_rodatstart);

  /* Allocate read/write memory */
  ret = vm_alloc(vmi, (void *)se.e_datstart,
                 se.e_bssstart - se.e_datstart + se.e_bsslen,
                 PG_TBL_PRESENT | PG_TBL_USER | PG_TBL_WRITABLE);
  assert(ret != NULL);

  /* Load read/execute sections (data and bss) */
  load_segment(filename, se.e_datoff, se.e_datlen, se.e_datstart);
  memset((void *)(se.e_datstart + se.e_datlen), 0,
         (se.e_bssstart + se.e_bsslen) - (se.e_datstart + se.e_datlen));

  return (void *)(se.e_entry);
}

/** @brief Loads single segment into memory.
 *.
 *  @param filename Name of file with segment.
 *  @param offset Offset of segment in file.
 *  @param len Length of segment in bytes.
 *  @param start Start of segment virtual address.
 *
 *  @return Void.
 */
void load_segment(const char* filename, int offset, size_t len,
                  unsigned long start)
{
  char segment[len];

  /* Read segment from the 'image' */
  getbytes(filename, offset, len, segment);

  /* Copy segment to virtual page(s) */
  memcpy((void*)(start), segment, len);

  return;
}

/*@}*/
