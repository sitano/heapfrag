#ifndef HEAPFRAG_H
#define HEAPFRAG_H 1

#include "ruby.h"

void rb_objspace_each_objects(
    int (*callback)(void *start, void *end, size_t stride, void *data),
    void *data);

size_t rb_gc_stat(VALUE hash_or_sym);

#ifndef HEAP_PAGE_ALIGN_LOG
/* default tiny heap size: 16KB */
#define HEAP_PAGE_ALIGN_LOG 14
#endif
#define CEILDIV(i, mod) (((i) + (mod) - 1)/(mod))

typedef uintptr_t bits_t;
enum {
    BITS_SIZE = sizeof(bits_t),
    BITS_BITLENGTH = ( BITS_SIZE * CHAR_BIT )
};

enum {
  HEAP_PAGE_ALIGN = (1UL << HEAP_PAGE_ALIGN_LOG),
  HEAP_PAGE_ALIGN_MASK = (~(~0UL << HEAP_PAGE_ALIGN_LOG)),
  REQUIRED_SIZE_BY_MALLOC = (sizeof(size_t) * 5),
  HEAP_PAGE_SIZE = (HEAP_PAGE_ALIGN - REQUIRED_SIZE_BY_MALLOC),
  HEAP_PAGE_HEADER_SIZE = sizeof(void *),
  RVALUE_SIZE = 40,
  HEAP_PAGE_OBJ_LIMIT = (unsigned int)((HEAP_PAGE_SIZE - HEAP_PAGE_HEADER_SIZE)/RVALUE_SIZE),
  HEAP_PAGE_BITMAP_LIMIT = CEILDIV(CEILDIV(HEAP_PAGE_SIZE, RVALUE_SIZE), BITS_BITLENGTH)
};

struct list_node
{
	struct list_node *next, *prev;
};

struct heap_page {
    short total_slots;
    short free_slots;
    short pinned_slots;
    short final_slots;
    struct {
	unsigned int before_sweep : 1;
	unsigned int has_remembered_objects : 1;
	unsigned int has_uncollectible_shady_objects : 1;
	unsigned int in_tomb : 1;
    } flags;

    struct heap_page *free_next;
    void *start;
    void *freelist;
    struct list_node page_node;

#if USE_RGENGC
    bits_t wb_unprotected_bits[HEAP_PAGE_BITMAP_LIMIT];
#endif
    /* the following three bitmaps are cleared at the beginning of full GC */
    bits_t mark_bits[HEAP_PAGE_BITMAP_LIMIT];
#if USE_RGENGC
    bits_t uncollectible_bits[HEAP_PAGE_BITMAP_LIMIT];
    bits_t marking_bits[HEAP_PAGE_BITMAP_LIMIT];
#endif

    /* If set, the object is not movable */
    bits_t pinned_bits[HEAP_PAGE_BITMAP_LIMIT];
};

struct heap_page_header {
    struct heap_page *page;
};

struct heap_page_body {
    struct heap_page_header header;
    /* char gap[];      */
    /* RVALUE values[]; */
};

#define GET_PAGE_BODY(x)   ((struct heap_page_body *)((bits_t)(x) & ~(HEAP_PAGE_ALIGN_MASK)))
#define GET_PAGE_HEADER(x) (&GET_PAGE_BODY(x)->header)
#define GET_HEAP_PAGE(x)   (GET_PAGE_HEADER(x)->page)

#define NUM_IN_PAGE(p)   (((bits_t)(p) & HEAP_PAGE_ALIGN_MASK)/RVALUE_SIZE)
#define BITMAP_INDEX(p)  (NUM_IN_PAGE(p) / BITS_BITLENGTH )
#define BITMAP_OFFSET(p) (NUM_IN_PAGE(p) & (BITS_BITLENGTH-1))
#define BITMAP_BIT(p)    ((bits_t)1 << BITMAP_OFFSET(p))

/* Bitmap Operations */
#define MARKED_IN_BITMAP(bits, p)    ((bits)[BITMAP_INDEX(p)] & BITMAP_BIT(p))
#define MARK_IN_BITMAP(bits, p)      ((bits)[BITMAP_INDEX(p)] = (bits)[BITMAP_INDEX(p)] | BITMAP_BIT(p))
#define CLEAR_IN_BITMAP(bits, p)     ((bits)[BITMAP_INDEX(p)] = (bits)[BITMAP_INDEX(p)] & ~BITMAP_BIT(p))

#if USE_RGENGC
#define GET_HEAP_WB_UNPROTECTED_BITS(x) (&GET_HEAP_PAGE(x)->wb_unprotected_bits[0])

#define RVALUE_WB_UNPROTECTED_BITMAP(obj) MARKED_IN_BITMAP(GET_HEAP_WB_UNPROTECTED_BITS(obj), (obj))

#define RVALUE_PAGE_WB_UNPROTECTED(page, obj) MARKED_IN_BITMAP(page->wb_unprotected_bits, (obj))

static inline int
RVALUE_WB_UNPROTECTED(VALUE obj) {
  return RVALUE_WB_UNPROTECTED_BITMAP(obj) != 0;
}
#endif


#endif
