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
 *  @param offset The location in the file to begin copying from.
 *  @param size The number of bytes to be copied.
 *  @param buf The buffer to copy the data into.
 *.
 *  @return The number of bytes copied on succes; -1 on failure.
 */
int getbytes(const char* filename, int offset, int size, char *buf )
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
    return -1;

  /* Copy size bytes from file starting at offset */
  src = (void *)&exec2obj_userapp_TOC[i].execbytes[offset];
  memcpy(buf, src, size);

  return size;
}
/** @brief Validate a file
 *
 *  Checks:
 *  1) ELF headers
 *  2) File is executable ELF binary
 *  3) File is actually in the ELF
 *
 *  @param filename Name of file to be validated
 *  @return int -1 on error otherwise 0
 *
 *  @bug This function is redundant with the work done in get_bytes() and
 *  load_file(), however get_bytes has a predefined prototype that 410kern/elf
 *  functions expect, and while we could parameterize validate file to take an
 *  se and be called from load_file, I don't want to because I wrote this
 *  function in order to validate the file given to exec before it dumps the
 *  existing program's memory and don't really want exec to be dealing with elf
 *  structs, although that could be changed at some point...
 */
int validate_file(const char* filename)
{
  simple_elf_t se;
  int i;

  /* Validate header and populate elf struct */
  if((elf_check_header(filename) == ELF_NOTELF) || 
      (elf_load_helper(&se,filename) == ELF_NOTELF)){
      return -1;
  }

  /* Search table of contents for file */
  for(i = 0; i < exec2obj_userapp_count; i++){
    if (0 == strcmp(filename,exec2obj_userapp_TOC[i].execname))
      break;
  }

  /* Error if we didn't find the file */
  if (i >= exec2obj_userapp_count)
    return -1;

  return 0;

}

/** @brief Loads file into memory 
 *
 *  Assumes page directory has already been initialized with 4 entries for
 *  kernel memory and one self-referential entry.
 *
 *  @param vmi Struct for keeping track of task's VM
 *  @param filename File to be loaded
 *
 *  @return The entry point of the newly loaded executable or NULL if the file 
 *  is invalid
 **/
void *load_file(vm_info_s *vmi, const char* filename)
{
  simple_elf_t se;
  void *ret;
  int i;

  /* Validate header and populate elf struct */
  if((elf_check_header(filename) == ELF_NOTELF) || 
      (elf_load_helper(&se,filename) == ELF_NOTELF)){
      return NULL;
  }

  /* Search table of contents for file */
  for(i = 0; i < exec2obj_userapp_count; i++){
    if (0 == strcmp(filename,exec2obj_userapp_TOC[i].execname))
      break;
  }

  /* Error if we didn't find the file */
  if (i >= exec2obj_userapp_count)
    return NULL;

  /* For simplicity, we assume text < rodata and data < bss */
  assert(se.e_txtstart < se.e_rodatstart);
  assert(se.e_datstart < se.e_bssstart);

  /* Allocate read/execute memory */
  ret = vm_alloc(vmi, (void *)se.e_txtstart,
                 (se.e_rodatstart - se.e_txtstart) + se.e_rodatlen,
                 VM_ATTR_USER);
  assert(ret != NULL);

  /* Load read/execute sections (text and rodata) */
  if(0 > getbytes(filename, se.e_txtoff, se.e_txtlen, (void *)se.e_txtstart))
    return NULL;

  memset((void *)(se.e_txtstart + se.e_txtlen), 0,
         se.e_rodatstart - (se.e_txtstart + se.e_txtlen));
  getbytes(filename, se.e_rodatoff, se.e_rodatlen, (void *)se.e_rodatstart);

  /* Allocate read/write memory */
  ret = vm_alloc(vmi, (void *)se.e_datstart,
                 se.e_bssstart - se.e_datstart + se.e_bsslen,
                 VM_ATTR_USER | VM_ATTR_RDWR);
  assert(ret != NULL);

  /* Load read/execute sections (data and bss) */
  getbytes(filename, se.e_datoff, se.e_datlen, (void *)se.e_datstart);
  memset((void *)(se.e_datstart + se.e_datlen), 0,
         (se.e_bssstart + se.e_bsslen) - (se.e_datstart + se.e_datlen));

  return (void *)(se.e_entry);
}

