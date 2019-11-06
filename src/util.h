#ifndef UTIL_H
#define UTIL_H

#define MEM_TO_CHUNK(x) (chunk_ptr) ((char *) x - 2*sizeof(size_t))
#define CHUNK_TO_MEM(x) (void *) ((char *) x + 2*sizeof(size_t))
#define CSIZE(x, a, m, p) (((x) & ~7) | ((a) << 2) | ((m) << 1) | ((p) << 0))

#define GET_AMP_BITS(x) ((x) & 7)
#define SET_AMP_BITS(x, b) (((x) & ~7) | b)
#define GET_BIT(x, b) ((x >> b) & (1))
#define GET_SIZE_FROM_CSIZE(x) ((x) & ~7)

#define sbin_index(x) ((x >> 3) - 2)
// #define BBIN_INDEX_FROM_SIZE(x) ((x >> 3) - 2)

#define IS_SMALL(x) (x <= ((SBIN_COUNT + 1) << 3))

// But it also smaller than some number
#define IS_LARGE(x) (x >  ((SBIN_COUNT + 1) << 3))
#define IS_EMPTY(x) ((x.next) == &(x))

#define is_mmaped_chunk(x) ((x)->csize & 2)

#define prev_chunk_in_use(x) ((x)->csize & 1)
#define prev_chunk(x) ((chunk_ptr) (((char *) (x)) - ((x)->psize) - sizeof(size_t) ))
#define next_chunk(x) ((chunk_ptr) (((char *) (x)) + (GET_SIZE_FROM_CSIZE((x)->csize)) +  sizeof(size_t)))

#define likely(x)   __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)

#define next_multiple_of(x, m) (((x) + ((m)-1)) & ~((m)-1))

#define gettid() (syscall(SYS_gettid))

#define in_main_thread() (gettid()==getpid())

/*
  https://stackoverflow.com/questions/2022179/ 
  This function rounds to the nearest upper multiple of 8, for example 29 will return 32
*/
static inline size_t correct_req_size(size_t req_size) 
{
  return req_size > 24 ? (req_size + 7) & ~7 : 24;
}

#endif
