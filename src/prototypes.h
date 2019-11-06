#ifndef PROTOTYPES_H
#define PROTOTYPES_H

static int bbin_index(size_t size);

static inline size_t correct_req_size(size_t req_size);
static inline chunk_ptr split_from_top(size_t size, arena_ptr a);
static inline void _unlink_(chunk_ptr chunk);
static inline void _link_(chunk_ptr prev, chunk_ptr chunk);

static inline void split_chunk(size_t split_size, chunk_ptr *c, chunk_ptr *t);
static inline void bin(chunk_ptr chunk, arena_ptr a);
static inline chunk_ptr sort_ubin(size_t request_size, arena_ptr a);
static inline chunk_ptr find_small_chunk(size_t request_size, arena_ptr a);

static inline chunk_ptr coallesce(chunk_ptr chunk, arena_ptr a);
static inline chunk_ptr _coallesce_(chunk_ptr prev, chunk_ptr next);
static inline chunk_ptr coallesce_unlink_prev(chunk_ptr chunk);
static inline chunk_ptr coallesce_unlink_next(chunk_ptr chunk, arena_ptr a);

static inline void _internal_aloc_(chunk_ptr chunk);
static inline void _internal_free_(chunk_ptr chunk);

static inline void *get_page(size_t size);
static inline void *get_page_size_aligned();
static inline chunk_ptr mmap_chunk(size_t request_size);
static inline void munmap_chunk(chunk_ptr chunk);

static inline void init_bins(arena_ptr arena);
static inline void myalloc_initialize(); // Creates main arena
static inline arena_ptr create_thread_arena(); // Creates thread arena
static inline iheap_ptr create_thread_iheap(arena_ptr arena);
static inline chunk_ptr _internal_free_top_(arena_ptr arena);
static inline arena_ptr locate_and_lock_arena();
static inline arena_ptr locate_arena();
static inline void acquire_arena(arena_ptr a);
static inline void release_arena(arena_ptr a);
static inline void append_arena(arena_ptr appendee);


#endif
