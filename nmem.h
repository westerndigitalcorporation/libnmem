#ifndef __NMEM_H
#define __NMEM_H

#include <stdlib.h>

int init_nmallocator(int node);
void *nmalloc(size_t);
void nmfree(void *);
void *nmrealloc(void *ptr, size_t size);

#endif
