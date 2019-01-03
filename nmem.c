#include "nmem.h"

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <numa.h>

static int numa_node = -1;

#define GUARD_PATTERN (0x8D8D8D8D)

typedef struct numa_mem {
    size_t size;
    unsigned int own;
} numa_mem_t;

/* Init the numa memory allocator */
int init_nmallocator(int node)
{
    int ret = numa_available();
    if (ret < 0 ) {
        goto err;
    }
    /* Ensure that the asked node is less than available numa nodes */
    if (node < 0 || node > numa_max_node()) {
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
    if (!mem) {
	return NULL;
    }

    void *buffer = mem + sizeof(numa_mem_t);
    numa_mem_t *nmem = (numa_mem_t *) mem;
    nmem->size = total_size;
    nmem->own = GUARD_PATTERN;
    memset(buffer, 0, size);
    return buffer;
}

/* Free memory and zero it out */
void nmfree(void *buffer)
{
    size_t size;

    if (!buffer) {
        return;
    }
    numa_mem_t *mem = (numa_mem_t *) (buffer - sizeof(numa_mem_t));
    size = mem->size;
    if (mem->own != GUARD_PATTERN) {
	/* Not our memory! */
	printf("Passed non numa memory to nmfree!\n");
	assert(0);
	return;
    }
    memset(mem, 0, size);
    numa_free(mem, size);
}

/* Reallocate memory for given size */
void *nmrealloc(void *ptr, size_t size)
{
    if (!ptr) {
	return nmalloc(size);
    }

    void *buffer = nmalloc(size);
    if (!buffer) {
	return NULL;
    }

    numa_mem_t *mem = (numa_mem_t *) (ptr - sizeof(numa_mem_t));
    if (mem->size > size) {
	memcpy(buffer, ptr, size);
    } else {
	memcpy(buffer, ptr, mem->size);
    }
    nmfree(ptr);
    ptr = buffer;
    return buffer;
}
