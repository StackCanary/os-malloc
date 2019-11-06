#ifndef MYALLOC_H
#define MYALLOC_H

#include <stdint.h>
#include <pthread.h>

#define WORD sizeof(void *)
#define ALIGN_STRUCT __attribute__((aligned(WORD))

/*
 * Heavily Inspired by Glibc/Pt/Doug Lee Malloc 
 */

extern void *myalloc(int size);
extern void  myfree(void *ptr);

#define MMAP_THRESHOLD (1024 * 1024)
#define MAX_HEAP_SIZE  (1024 * 1024)
#define PAGE_SIZE (4096)
#define INITIAL_ALLOCATION_SIZE (PAGE_SIZE * 32)
#define EXTRA_ARENA_COUNT 4

/*
 * Size is aligned to 8 Bytes (Last Bit is used for checking if chunk above is free) in which case prev_size does not contain user data.
 *
 */
typedef struct chunk {
  size_t psize; // {A, M, P}
  size_t csize; 
  struct chunk *next;
  struct chunk *prev;
} chunk_t, *chunk_ptr;

#define SBIN_COUNT 63
#define BBIN_COUNT 63

//  If stuff is breaking, its because something is not aligned

/*
  
 */
typedef struct arena {
  chunk_ptr top;
  chunk_t ubin;
  chunk_t sbin[SBIN_COUNT];
  chunk_t bbin[BBIN_COUNT];
  struct arena *next;
  struct arena *free;
  pthread_mutex_t m;
} arena_t, *arena_ptr;


/*
  Only needed when you have multiple heaps, or heaps in the non-main arena
 */
typedef struct heapinfo {
  arena_ptr arena;
  struct heapinfo *prev;
  size_t hsize;
} heapinfo_t, *iheap_ptr;


// Debug prototypes
void print_arena (chunk_ptr c);
void print_chunk (chunk_ptr chunk);
void test();



#endif
