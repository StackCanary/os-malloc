#include "myalloc.h"

#include <assert.h>
#include <stdio.h>
#include <signal.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "util.h"

// This should initialize the other members to 0
static arena_t  arena = {.m = PTHREAD_MUTEX_INITIALIZER};
static arena_t *extra[EXTRA_ARENA_COUNT] = {0};
pthread_mutex_t global_initialisation_lock = PTHREAD_MUTEX_INITIALIZER; // Aqcuire this if initialising something
static pthread_mutex_t extra_arena_lock = PTHREAD_MUTEX_INITIALIZER; // Aqcuire this if initialising something

static pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

static int initialised = 0;

static int extra_arena_count = 0;


chunk_ptr reference = 0;

#define nullptr (NULL)

/* 
   Static Prototypes
*/
#include "prototypes.h"

#include "debug.h"
#include "chunk.h"
#include "arena.h"
#include "mpage.h"

void test()
{
  /* arena_ptr non_thread_arena = create_thread_arena(); */
  /* iheap_ptr non_thread_iheap = create_thread_iheap(non_thread_arena); */

  /* printf("Heap %p, %p\n", iheap_for_chunk(non_thread_iheap->arena->top), non_thread_iheap); */
}


void *myalloc(int size)
{
  if (size < 0)
    return nullptr;

  size_t request_size = correct_req_size(size);
  /*
    Initialize and construct initial giant chunk, make sure to acquire mutex
  */
  pthread_mutex_lock(&global_initialisation_lock);
  
  if (!initialised)
    myalloc_initialize();
  
  pthread_mutex_unlock(&global_initialisation_lock);

  /*
    If size >= 1Mib use mmap directly
  */
  if (size >= (1 << 20)) {
    chunk_ptr cmap = mmap_chunk(request_size);
    void *mem = CHUNK_TO_MEM(cmap);
    return mem;
  }

  /* Try to find arena to service and acquire lock on arena, each thread gets assigned an arena to deal with thread contention */
  arena_ptr a = locate_and_lock_arena(); // locate_arena
  chunk_ptr c = nullptr;

  /* Around here goes the malloc algorithm */

  if ((c = find_small_chunk(request_size, a))) 
    goto success;

  if ((c = sort_ubin(request_size, a)))
    goto success;

  if ((c = find_large_chunk(request_size, a)))
    goto success;

  // If can't find suitable chunk in break top chunk into two
  if ((c = split_from_top(request_size, a)))
    goto success;
  
  release_arena(a);

 failure:
  return nullptr;

 success:
  _internal_aloc_(c);
  release_arena(a);
  return CHUNK_TO_MEM(c);
}


/*
  Set the chunk's next's csize's free bit on and set thread arena bit
*/
static inline void _internal_aloc_(chunk_ptr chunk)
{
  next_chunk(chunk)->csize |= 1;

  if (!in_main_thread())
    chunk->csize |= 4;
}

/*
  Set the chunk's next's csize's free bit off 
  Since the chunk is free'd, we can store the prev size for easier coallescing.
*/
static inline void _internal_free_(chunk_ptr chunk)
{
  next_chunk(chunk)->csize &= ~1;
  next_chunk(chunk)->psize = GET_SIZE_FROM_CSIZE(chunk->csize);
}

/*
  Freeing a chunk is fast, the chunk is simply inserted into the unsorted bin
 */
void myfree(void *mem)
{
  chunk_ptr c = MEM_TO_CHUNK(mem);

  // If mmap was directly used to allocate this chunk, then munmap it, don't need to acquire lock
  if (is_mmaped_chunk(c)) {
    munmap_chunk(c);
    return;
  }

  // Find arena that this chunk belongs to
  arena_ptr a = arena_for_chunk(c);

  // Acquire a lock on this arena
  acquire_arena(a);

  c = coallesce(c, a);

  // Coallesce with arena.top 
  if (next_chunk(c) == a->top) {
    _internal_free_(c);
    a->top = _coallesce_(c, a->top);
    goto finish;
  }

  // Insert into unsorted bin
  _internal_free_(c);
  _link_(&a->ubin, c);
  goto finish;

  // Release lock
 finish:
  release_arena(a);
  return;

}





