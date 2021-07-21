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

enum {
  HEAP_PAGE_ALIGN = (1UL << HEAP_PAGE_ALIGN_LOG),
  HEAP_PAGE_ALIGN_MASK = (~(~0UL << HEAP_PAGE_ALIGN_LOG)),
  REQUIRED_SIZE_BY_MALLOC = (sizeof(size_t) * 5),
  HEAP_PAGE_SIZE = (HEAP_PAGE_ALIGN - REQUIRED_SIZE_BY_MALLOC),
  HEAP_PAGE_HEADER_SIZE = sizeof(void *),
  RVALUE_SIZE = 40,
  HEAP_PAGE_OBJ_LIMIT = (unsigned int)((HEAP_PAGE_SIZE - HEAP_PAGE_HEADER_SIZE)/RVALUE_SIZE)
};

#endif