#ifndef CHUNK_H
#define CHUNK_H

static int bbin_index(size_t size)
{
  if (size < 512 + (1 << 11)) {
    return (size - 512) >> 6;
  }
  else if(size < 2560 + (1 << 13)) {
    return 31 + ((size - 2560) >> 9);
  }
  else if (size < 10752 + (1 << 15)) {
    return 47 + ((size - 10752) >> 12);
  }
  else if (size < 43520 + (1 << 17)) {
    return 55 + ((size - 43520) >> 15);
  }
  else if (size < 174592 + (1 << 19)) {
    return 59 + ((size - 174592) >> 18);
  }
  else {
    return BBIN_COUNT - 1;
  }
}

static inline void split_chunk(size_t split_size, chunk_ptr *c, chunk_ptr *t)
{
  
  size_t bits = GET_AMP_BITS((*c)->csize);
  size_t size = GET_SIZE_FROM_CSIZE((*c)->csize);

  if (split_size + 64 >= size) {
    *t = nullptr;
    return;
  }

  (*c)->csize = ((split_size) | bits);
  
  (*t) = (chunk_ptr) (((char *) (*c)) + sizeof(size_t) + split_size);
  (*t)->csize =  ((size - split_size - sizeof(size_t)) | bits);
  (*t)->psize = GET_SIZE_FROM_CSIZE((*c)->csize);

  _internal_free_(*t);
}

static inline chunk_ptr sort_ubin(size_t request_size, arena_ptr a)
{

  if (IS_EMPTY(a->ubin))
    return nullptr;

  chunk_ptr c = &a->ubin;
  while(c != &a->ubin) {

    if (GET_SIZE_FROM_CSIZE(c->csize) >= request_size) {
      
      _unlink_(c);

      chunk_ptr t = nullptr;
      split_chunk(request_size, &c, &t);

      //  On a successful split, reinsert the unused split part of the chunk back into ubin
      if (t)
    	_link_(&a->ubin, t);

      return c;
    }

    chunk_ptr n = c->next;
    
    // Coallesce and inserto into small / large bins
    _unlink_(c);
    c = coallesce(c, a);
    bin(c, a);

    c = n;
  }
  
  return nullptr;
}

static inline chunk_ptr find_small_chunk(size_t request_size, arena_ptr a)
{
  if (IS_SMALL(request_size)) {
    
    int index = sbin_index(request_size);

    if ( !(IS_EMPTY(a->sbin[index])) ) {
      chunk_ptr c = a->sbin[index].next;
      _unlink_(c);
      return c;
    }
    
  }

  return nullptr;
}


static inline chunk_ptr find_large_chunk(size_t request_size, arena_ptr a)
{
  for (int index = bbin_index(request_size); index < BBIN_COUNT; index++ ) {
    
    if (!IS_EMPTY(a->bbin[index]) ) {

      chunk_ptr c = &a->bbin[index];
      while((c = c->next) != &a->bbin[index]) {
	if (GET_SIZE_FROM_CSIZE(c->csize) >= request_size) {
	  _unlink_(c);

	  chunk_ptr t = nullptr;
	  split_chunk(request_size, &c, &t);

	  // On a successful split, reinsert the unused split part of the chunk back into ubin
	  if (t)
	    _link_(&a->ubin, t);
	  
	  return c;
	}

      }
      
    }
    
  }
    
  return nullptr;
}


// Uses an alignment trick which I learned from glibc malloc 
static inline iheap_ptr iheap_for_chunk(chunk_ptr chunk)
{
  return (iheap_ptr) ((unsigned long) chunk & ~(MAX_HEAP_SIZE - 1));;
}

static inline arena_ptr arena_for_chunk(chunk_ptr chunk)
{
  // if main arena
  if (!GET_BIT(chunk->csize, 2))
    return &arena; // return main arena
  else
    return iheap_for_chunk(chunk)->arena;
}


// TODO
static inline void bin(chunk_ptr chunk, arena_ptr a)
{
  size_t size = GET_SIZE_FROM_CSIZE(chunk->csize);

  if (IS_SMALL(size)) {
    _link_(&a->sbin[sbin_index(size)], chunk);
  }
  else { // Must be Large chunk
    _link_(&a->bbin[bbin_index(size)], chunk);
  }
  
}

/*
  Coallesce adjacent chunks
*/
static inline chunk_ptr coallesce(chunk_ptr chunk, arena_ptr a)
{
  chunk = coallesce_unlink_prev(chunk);
  chunk = coallesce_unlink_next(chunk, a);
  
  return chunk;
}


/*
  Coallesce the prev with the chunk, returning the coallesced chunk
*/
static inline chunk_ptr coallesce_unlink_prev(chunk_ptr chunk)
{
  
  if (!prev_chunk_in_use(chunk)) {
    _unlink_(prev_chunk(chunk));
    return _coallesce_(prev_chunk(chunk), chunk);
  }
  
  return chunk;
}

/* 
   Coallesce the next with the chunk, returning the coallesced chunk
*/
static inline chunk_ptr coallesce_unlink_next(chunk_ptr chunk, arena_ptr a)
{

  chunk_ptr next = next_chunk(chunk);

  // Top chunk
  if (next == a->top)
    return chunk;

  // Barrier chunk
  if (GET_SIZE_FROM_CSIZE(next->csize) == 0)
    return chunk;

  if (!prev_chunk_in_use(next_chunk(next))) {
    _unlink_(next);
    return _coallesce_(chunk, next);
  }
  
  return chunk;
}

/*
  A helper function which performs the size calculations to produce coallesced chunk
*/
static inline chunk_ptr _coallesce_(chunk_ptr prev, chunk_ptr next)
{
  // Keep 3 lsb of prev chunk only
  int lsb = GET_AMP_BITS(prev->csize);

  size_t p_size = GET_SIZE_FROM_CSIZE(prev->csize);
  size_t n_size = GET_SIZE_FROM_CSIZE(next->csize);
  
  prev->csize = SET_AMP_BITS(p_size + n_size + sizeof(size_t), lsb);
  return prev;
}


static inline void _link_(chunk_ptr prev, chunk_ptr chunk) {
  
  chunk->next = prev->next;
  chunk->prev = prev;

  if (prev->next)
    prev->next->prev = chunk;
  
  prev->next = chunk;
}
  
static inline void _unlink_(chunk_ptr chunk)
{
  chunk_ptr next = chunk->next;
  chunk_ptr prev = chunk->prev;
  
  if (next)
    next->prev = prev;
  
  prev->next = next;
}

#endif

