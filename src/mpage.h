#ifndef MPAGE_H
#define MPAGE_H

static inline void *get_page(size_t size)
{
  return mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

/*
  Another trick used by glibc malloc to get a size aligned page
  It allocates a page that is twice as big and uses the nearest upper multiple trick
  Glibc malloc also remembers the next aligned region if it is allocatable, but I won't 
  use that for now.
  https://github.com/sploitfun/lsploits/blob/master/glibc/malloc/arena.c#L59
 */
static inline void *get_page_size_aligned()
{
  char *p1 = mmap(nullptr, MAX_HEAP_SIZE << 1, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); // Allocate twice as big page

  // Align to size
  char *p2 = (char *) (((unsigned long) p1 + (MAX_HEAP_SIZE - 1)) & ~(MAX_HEAP_SIZE - 1));

  // p2 >= p1

  char *p3 = p2 + (MAX_HEAP_SIZE);
  char *tp = p1 + (MAX_HEAP_SIZE << 1);

  unsigned long df = p2 - p1;
  
  if (df)
    munmap(p1, df);
  /* else
      ... char * next_heap = p2 + MAX_HEAP_SIZE
      // This is what I would have if I wanted to save the next aligned region
   */

  munmap(p3, (unsigned long) tp - (unsigned long) p3);
  
  return p2;
}

static inline chunk_ptr mmap_chunk(size_t request_size)
{
  size_t size = next_multiple_of((request_size + sizeof(size_t)), PAGE_SIZE);
  chunk_ptr chunk = (chunk_ptr) ((char *) get_page(size) - sizeof(size_t));
  chunk->csize = (size - 2 * sizeof(size_t)) | 2;
  return chunk;
}

static inline void munmap_chunk(chunk_ptr chunk)
{
  munmap((char *) chunk + sizeof(size_t), GET_SIZE_FROM_CSIZE(chunk->csize) + sizeof(size_t));
}

#endif
