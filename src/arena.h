#ifndef ARENA_H
#define ARENA_H

static inline void init_bins(arena_ptr arena)
{
  arena->ubin.next = arena->ubin.prev = &(arena->ubin);

  for (int i = 0; i < SBIN_COUNT; i++)
    arena->sbin[i].next = arena->sbin[i].prev = &(arena->sbin[i]);

  for (int i = 0; i < BBIN_COUNT; i++)
    arena->bbin[i].next = arena->bbin[i].prev = &(arena->bbin[i]);
}

static inline void myalloc_initialize()
{
  arena.top = (chunk_ptr) ((char *) sbrk(0) - sizeof(arena.top->psize));
  sbrk(INITIAL_ALLOCATION_SIZE);
  arena.top->csize = CSIZE((char *) sbrk(0) - (char *) arena.top - sizeof(size_t), 0, 0, 1); // -8 here
  arena.top->next = nullptr;
  arena.top->prev = nullptr;

  init_bins(&arena);

  initialised++;
}

static inline arena_ptr locate_and_lock_arena()
{

  arena_ptr a = locate_arena();
  acquire_arena(a);
  
  return a;
}

// finds
static inline arena_ptr locate_arena()
{
  
  if (in_main_thread())
    return &arena;

  pthread_mutex_lock(&extra_arena_lock);

  long index = gettid() % (long) EXTRA_ARENA_COUNT;

  //  printf("Tid %ld, Index %ld\n", gettid(), index);

  // If arena doesn't exist, create it
  if (extra[index] == nullptr)
    extra[index] = create_thread_arena();

  arena_ptr a = extra[index];

  pthread_mutex_unlock(&extra_arena_lock);
  
  return a;
}

static inline void acquire_arena(arena_ptr a)
{
  pthread_mutex_lock(&a->m);
}

// releases arena
static inline void release_arena(arena_ptr a)
{
  pthread_mutex_unlock(&a->m);
}

static inline void append_arena(arena_ptr appendee)
{
  arena_ptr conductor = &arena;

  pthread_mutex_lock(&list_lock);

  while(conductor->next)
    conductor = conductor->next;

  conductor->next = appendee;

  pthread_mutex_unlock(&list_lock);
  
}


static inline arena_ptr create_thread_arena()
{
  size_t hsize = INITIAL_ALLOCATION_SIZE;

  void *page = get_page_size_aligned();
  mprotect(page, INITIAL_ALLOCATION_SIZE, PROT_READ | PROT_WRITE);
  
  iheap_ptr iheap = page;
  arena_ptr a = (arena_ptr) (sizeof(heapinfo_t) + ((char *) page));
  
  iheap->hsize = hsize;
  iheap->arena = a;

  a->top = (chunk_ptr) (((char *) a) + sizeof(arena_t) - sizeof(size_t));
  a->top->csize = CSIZE(hsize - sizeof(arena_t) - sizeof(heapinfo_t) - sizeof(size_t), 1, 0, 1);
  a->top->next = nullptr;
  a->top->prev = nullptr;
  pthread_mutex_init(&a->m, nullptr);

  init_bins(a);

  append_arena(a);
  extra_arena_count++; // Increment this to keep track of extra arenas
  
  return a;
}


// TODO the prev pointer in heap data structure
static inline iheap_ptr create_thread_iheap(arena_ptr a)
{
  size_t size = INITIAL_ALLOCATION_SIZE;
  
  void  *page = get_page_size_aligned();
  mprotect(page, INITIAL_ALLOCATION_SIZE, PROT_READ | PROT_WRITE);
  
  iheap_ptr heap = page;
  
  heap->hsize = size;
  heap->arena = a;

  // Construct a linked list of heaps
  heap->prev = iheap_for_chunk(a->top);

  // Split into Top chunk followed by a minimum sized chunk to store the prev is free bit
  chunk_ptr old_top = _internal_free_top_(a);
  bin(old_top, a);

  // Set top to point to the new top chunk and initialize it
  a->top = (chunk_ptr) (((char *) heap) + sizeof(heapinfo_t) - sizeof(size_t));
  a->top->csize = CSIZE(size - sizeof(heapinfo_t) - sizeof(size_t), 1, 0, 1); // Arena bit is set, Prev is in use is set
  a->top->next = nullptr;
  a->top->prev = nullptr;

  return heap;
}


// Might need to split into top followed by two tips
static inline chunk_ptr _internal_free_top_(arena_ptr arena)
{
  // Reduce size of top so we can place a chunk after it
  chunk_ptr top = arena->top;
  size_t bits = GET_AMP_BITS(top->csize);
  top->csize = (GET_SIZE_FROM_CSIZE(top->csize) - sizeof(size_t)) | bits;

  // Create a chunk with size 0 as the "tip", set prev is free and don't bin it
  chunk_ptr tip = next_chunk(top);
  tip->csize = CSIZE(0, GET_BIT(bits, 2), 0, 0);
  tip->psize = GET_SIZE_FROM_CSIZE(top->csize);

  return top;
}

#define TOP_RESERVE 4096

static inline void _extend_heap_(arena_ptr a, size_t size)
{
  iheap_ptr heap = iheap_for_chunk(a->top);
  mprotect(heap, heap->hsize + size, PROT_READ | PROT_WRITE);
  heap->hsize += size;
  a->top->csize += size;
}

// TODO creates a new one, could also extend it a feature for the future
static inline void extend_heap(arena_ptr a, size_t size)
{

  iheap_ptr heap = iheap_for_chunk(a->top);

  if (heap->hsize + size < MAX_HEAP_SIZE) {
    _extend_heap_(a, size);
  }
  else {
    create_thread_iheap(a);

    heap = iheap_for_chunk(a->top);
    
    if (GET_SIZE_FROM_CSIZE(a->top->csize) < size )
      _extend_heap_(a, size);
  }
  
}

// TODO
// This function deals with both cases in split from top
static inline void extend_top(arena_ptr a, int size)
{
  // is main arena
  if (a == &arena) {
    // sbrk more memory
    sbrk(size);
    a->top->csize += size;
  } else {
    // extend_heap or create new heap
    extend_heap(a, size);
  }  

}

static inline chunk_ptr split_from_top(size_t size, arena_ptr a)
{

  // Check if we have enough space
  // TODO Doesn't deal with a main arena top chunk vs thread arena top chunk

  size_t size_needed = size + TOP_RESERVE;
  if (size_needed > a->top->csize) {
    extend_top(a, size_needed);
  }

  chunk_ptr result = a->top;
  a->top = (chunk_ptr) ((char *) a->top + sizeof(size_t) + size);
  a->top->csize = CSIZE(result->csize - sizeof(size_t) - size, 0, 0, 1);
  a->top->next = nullptr;
  a->top->prev = nullptr;

  result->csize = CSIZE(size, a!=&arena, 0, 1);
    
  return result;
}

#endif
