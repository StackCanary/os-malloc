#ifndef DEBUG_H
#define DEBUG_H

static pthread_mutex_t debug_lock = PTHREAD_MUTEX_INITIALIZER;

/*
  Functions for debugging
 */
void print_arena (chunk_ptr c)
{
  pthread_mutex_lock(&debug_lock);
  
  long index = gettid() % (long) EXTRA_ARENA_COUNT;

  arena_ptr a = nullptr;

  if (in_main_thread())
    a = &arena;
  else
    a = extra[index];
  

  if (in_main_thread())
    printf("Arena: main\n");
  else
    printf("Arena: %ld\n", index);

  for (chunk_ptr next = c; ;  next = next_chunk(next)) {
    
    print_chunk(next);
    
    if (next == a->top)
      break;
  }

  pthread_mutex_unlock(&debug_lock);

}

void print_chunk (chunk_ptr chunk)
{

  long index = gettid() % (long) EXTRA_ARENA_COUNT;
  arena_ptr a = nullptr;

  if (in_main_thread())
    a = &arena;
  else
    a = extra[index];
  
  printf("\n");
  printf("addr %p\n",  chunk);

  if (!is_mmaped_chunk(chunk))
    printf("p_size %ld\n", chunk->psize);
  printf("size %lu\n", chunk->csize);
  printf("next %p\n", chunk->next);
  printf("prev %p\n", chunk->prev);

  if (chunk != a->top) 
    printf("Status: %s\n", prev_chunk_in_use(next_chunk(chunk)) ? "Allocated" : "Free");
  else
    printf("Status: Top Chunk\n");
  
  printf("\n");
}

#endif
