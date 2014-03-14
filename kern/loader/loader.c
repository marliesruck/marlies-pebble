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
#include <util.h>

#include <pg_table.h>
#include <frame_alloc.h>
#include <page_alloc.h>

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
 *  @param se Elf struct to populate
 *  @param filename Name of file to be validated
 *  @return int -1 on error otherwise the index into the table of contents
 */
int validate_file(simple_elf_t *se, const char* filename)
{
  int i;
  /* Validate header and populate elf struct */
  if((elf_check_header(filename) == ELF_NOTELF) || 
      (elf_load_helper(se,filename) == ELF_NOTELF)){
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
/* @brief Loads data and bss.
 *
 * Data and bss have a separate because data's location in memory will affect
 * bss.  If data and bss overlap on a page, then the bss portion is simply
 * initialized to zeroes. Moreover, if bss spans multiple pages, then those
 * remaining PTE should be ZFOD.  
 *
 * @bug Right now memory between regions is zeroed out but also user accessible
 * when in fact this should cause a page fault.  More fine grained permissions?
 *
 * @param vmi Struct for keeping track of task's VM.
 * @param filename File to be loaded.
 * @param se ELF information for loading file.
 */
void copy_data_bss(vm_info_s *vmi, const char *filename, simple_elf_t *se)
{
  void *ret;
  mem_region_s *mreg;

  unsigned int bss_start = FLOOR(se->e_bssstart, PAGE_SIZE);
  unsigned int dat_limit = CEILING(se->e_datstart + se->e_datlen, PAGE_SIZE);
  unsigned int bss_limit = CEILING(se->e_bssstart + se->e_bsslen, PAGE_SIZE);

  /* Data and BSS share a page */
  if(bss_start <= dat_limit){

    /* Allocate 1 region for data and BSS */
    ret = vm_alloc(vmi, (void *)se->e_datstart, se->e_datlen, 
                   VM_ATTR_USER | VM_ATTR_RDWR);
    assert(ret != NULL);

    /* Zero non-data part of page (which is BSS and possibly a 'hole' between
     * regions  */
    memset((void *)(se->e_datstart + se->e_datlen), 0,dat_limit - 
            (se->e_datstart + se->e_datlen));

    /* Allocate a ZFOD region for remaining BSS */
    if(dat_limit != bss_limit){
      unsigned int bss_ceiling = CEILING(se->e_bssstart, PAGE_SIZE);
      mreg = vm_region(vmi, (void *)bss_ceiling, bss_limit - bss_ceiling,
                       VM_ATTR_USER | VM_ATTR_ZFOD);
      assert(mreg != NULL);

      /* Set PTEs to ZFOD */
      vm_zfod(mreg,&vmi->pg_info);
    }
  }
  else{
    /* Allocate 1 region for ONLY data */
    ret = vm_alloc(vmi, (void *)se->e_datstart, se->e_datlen, 
                   VM_ATTR_USER | VM_ATTR_RDWR);
    assert(ret != NULL);

    /* Allocate a region for bss starting with the end of data*/
    mreg = vm_region(vmi, (void *)dat_limit, bss_limit - dat_limit,
                     VM_ATTR_USER | VM_ATTR_ZFOD);
    assert(mreg != NULL);

    /* Set PTEs to ZFOD */
    vm_zfod(mreg,&vmi->pg_info);
  }

  /* Load read/execute sections (data) */
  getbytes(filename, se->e_datoff, se->e_datlen, (void *)se->e_datstart);
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

  /* Validate file and populate elf struct */
  if(validate_file(&se,filename) < 0)
    return NULL;

  /* For simplicity, we assume text < rodata and data < bss */
  assert(se.e_txtstart < se.e_rodatstart);

  assert(se.e_datlen);
  assert(se.e_bsslen);

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

  copy_data_bss(vmi, filename, &se);

  return (void *)(se.e_entry);
}

