/* The 15-410 kernel project
 *
 *     loader.h
 *
 * Structure definitions, #defines, and function prototypes
 * for the user process loader.
 */

#ifndef _LOADER_H
#define _LOADER_H
     

/* --- Prototypes --- */

int getbytes( const char *filename, int offset, int size, char *buf );

/*
 * Declare your loader prototypes here.
 */

void load_file(const char* filename);
void load_segment(const char* filename, unsigned long offset, 
                  unsigned long len, unsigned long start, unsigned int flags);

#endif /* _LOADER_H */
