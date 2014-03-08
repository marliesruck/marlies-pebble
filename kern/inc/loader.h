/* @file loader.h
 *
 * Structure definitions, #defines, and function prototypes
 * for the user process loader.
 */

#ifndef _LOADER_H
#define _LOADER_H
     
#include <vm.h>


int getbytes(const char *filename, int offset, int size, char *buf);
void *load_file(vm_info_s *vmi, const char* filename);
int validate_file(const char* filename);


#endif /* _LOADER_H */

