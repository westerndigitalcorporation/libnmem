#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <numa.h>
#include "nmem.h"

static int numa_node = -1;

typedef struct numa_mem {
    size_t size;
    void *mem;
} numa_mem_t;

/* Init the numa memory allocator */
int init_nmallocator(int node)
{
    int ret = numa_available();
    if (ret < 0 ) {
        goto err;
    }
    /* Ensure that the asked node is less than available numa nodes */
    if (node > numa_max_node()) {
	ret = -1;
	goto err;
    }
    numa_node = node;
err:
    return ret;
}

/* Allocate memory and zero it out */
void *nmalloc(size_t size)
{
    size_t total_size = size + sizeof(numa_mem_t);
    
    void *mem = numa_alloc_onnode(size + total_size, numa_node);
    assert(mem);

    void *buffer = mem + sizeof(numa_mem_t);
    numa_mem_t *nmem = (numa_mem_t *) mem;
    nmem->size = total_size;
    nmem->mem = buffer;
    memset(nmem->mem, 0, size);
    return nmem->mem;
}

/* Free memory and zero it out */
void nmfree(void *buffer)
{
    size_t size;

    numa_mem_t *mem = (numa_mem_t *) (buffer - sizeof(numa_mem_t));
    assert(mem->size > 0);
    size = mem->size;
    memset(buffer, 0, size);
    numa_free(mem, size);
}
