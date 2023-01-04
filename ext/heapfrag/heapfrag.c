#include "heapfrag.h"

// #include <stdio.h>

VALUE rb_mHeapfrag;

VALUE sym_fragmented_pages;
VALUE sym_pages_total;
VALUE sym_pages_seen;
VALUE sym_pages_walive;
VALUE sym_pages_wdead;
VALUE sym_pages_wshady;
VALUE sym_pages_wuncol;
VALUE sym_pages_wremset;
VALUE sym_pages_frag_shady;
VALUE sym_pages_free;
VALUE sym_pages_full;
VALUE sym_alive_objs;
VALUE sym_dead_objs;
VALUE sym_shady_objs;
VALUE sym_heap_allocated_pages;
VALUE sym_pages_fill_cdf;

static size_t cdf_levels[] = {
  1,                                // 0
  HEAP_PAGE_OBJ_LIMIT * 50  / 100,  // 50%
  HEAP_PAGE_OBJ_LIMIT * 95  / 100,  // 95%
  HEAP_PAGE_OBJ_LIMIT * 99  / 100,  // 99%
  HEAP_PAGE_OBJ_LIMIT * 999 / 1000, // 99.9%
  HEAP_PAGE_OBJ_LIMIT               // inf+
};

struct heap_info {
  size_t pages_seen;
  size_t pages_walive;
  size_t pages_wdead;
  size_t pages_wshady;
  size_t pages_wuncol;
  size_t pages_wremset;
  size_t pages_frag_shady;
  size_t pages_free;
  size_t pages_full;
  size_t alive_objs;
  size_t dead_objs;
  size_t shady_objs; // write unprotected objects
  size_t cdf[sizeof(cdf_levels) / sizeof(size_t)];
};

static void
insert_cdf(struct heap_info *info, size_t val)
{
  static size_t size = sizeof(cdf_levels) / sizeof(size_t);

  for (size_t i = 0; i < size; i ++) {
    if (val < cdf_levels[i]) {
      info->cdf[i] ++;
    }
  }

  if (val >= cdf_levels[size - 1]) {
    info->cdf[size - 1] ++;
  }
}

static int
count_objects_i(void *vstart, void *vend, size_t stride, void *data)
{
  struct heap_info *info = (struct  heap_info *)data;
  VALUE v = (VALUE)vstart;
  size_t alive = 0;
  size_t dead = 0;
  size_t shady = 0;

  for (; v != (VALUE)vend; v += stride) {
    if (RBASIC(v)->flags) {
      alive ++;
    } else {
      dead ++;
    }

#if USE_RGENGC
      if (RVALUE_WB_UNPROTECTED(v)) {
        shady++;
      }
#endif
  }

  info->pages_wremset += (GET_HEAP_PAGE(vstart)->flags.has_remembered_objects) ? 1 : 0;
  info->pages_wuncol += (GET_HEAP_PAGE(vstart)->flags.has_uncollectible_shady_objects) ? 1 : 0;

  info->pages_seen ++;
  info->alive_objs += alive;
  info->dead_objs += dead;
  if (alive) {
    info->pages_walive ++;
  } else {
    info->pages_free ++;
  }
  if (dead) {
    info->pages_wdead ++;
  } else {
    info->pages_full ++;
  }
  if (shady) {
    info->pages_wshady ++;
    info->shady_objs += shady;
    if (alive && dead) info->pages_frag_shady ++;
  }

  insert_cdf(info, alive);

  // printf("%ld / %ld: %p - %p = (%ld)\n", alive, dead, vstart, vend, (vend-vstart)/40);

  return 0;
}

static VALUE
heapfrag_stat(VALUE mod)
{
  struct heap_info info = {};
  VALUE rv = rb_hash_new();
  size_t total_pages = rb_gc_stat(sym_heap_allocated_pages);

  rb_objspace_each_objects(count_objects_i, &info);
  rb_hash_aset(rv, sym_fragmented_pages, INT2FIX(total_pages - info.pages_full - info.pages_free));
  rb_hash_aset(rv, sym_pages_total, INT2FIX(total_pages));
  rb_hash_aset(rv, sym_pages_seen, INT2FIX(info.pages_seen));
  rb_hash_aset(rv, sym_pages_walive, INT2FIX(info.pages_walive));
  rb_hash_aset(rv, sym_pages_wdead, INT2FIX(info.pages_wdead));
  rb_hash_aset(rv, sym_pages_wshady, INT2FIX(info.pages_wshady));
  rb_hash_aset(rv, sym_pages_wuncol, INT2FIX(info.pages_wuncol));
  rb_hash_aset(rv, sym_pages_wremset, INT2FIX(info.pages_wremset));
  rb_hash_aset(rv, sym_pages_frag_shady, INT2FIX(info.pages_frag_shady));
  rb_hash_aset(rv, sym_pages_free, INT2FIX(info.pages_free));
  rb_hash_aset(rv, sym_pages_full, INT2FIX(info.pages_full));
  rb_hash_aset(rv, sym_alive_objs, INT2FIX(info.alive_objs));
  rb_hash_aset(rv, sym_dead_objs, INT2FIX(info.dead_objs));
  rb_hash_aset(rv, sym_shady_objs, INT2FIX(info.shady_objs));
  rb_hash_aset(rv, sym_pages_fill_cdf,
               rb_ary_new_from_args(6, INT2FIX(info.cdf[0]), INT2FIX(info.cdf[1]), INT2FIX(info.cdf[2]),
                                    INT2FIX(info.cdf[3]), INT2FIX(info.cdf[4]), INT2FIX(info.cdf[5])));

  return rv;
}

void
Init_heapfrag(void)
{
  rb_mHeapfrag = rb_define_module("Heapfrag");
  rb_define_singleton_method(rb_mHeapfrag, "stat", heapfrag_stat, 0);

  sym_fragmented_pages = ID2SYM(rb_intern("fragmented_pages"));
  sym_pages_total = ID2SYM(rb_intern("pages_total"));
  sym_pages_seen = ID2SYM(rb_intern("pages_seen"));
  sym_pages_walive = ID2SYM(rb_intern("pages_with_alive"));
  sym_pages_wdead = ID2SYM(rb_intern("pages_with_dead"));
  sym_pages_wshady = ID2SYM(rb_intern("pages_with_shady"));
  sym_pages_wuncol = ID2SYM(rb_intern("pages_with_uncol"));
  sym_pages_wremset = ID2SYM(rb_intern("pages_with_remset"));
  sym_pages_frag_shady = ID2SYM(rb_intern("fragmented_with_shady"));
  sym_pages_free = ID2SYM(rb_intern("pages_free"));
  sym_pages_full = ID2SYM(rb_intern("pages_full"));
  sym_alive_objs = ID2SYM(rb_intern("objs_alive"));
  sym_dead_objs = ID2SYM(rb_intern("objs_dead"));
  sym_shady_objs = ID2SYM(rb_intern("objs_shady"));
  sym_heap_allocated_pages = ID2SYM(rb_intern("heap_allocated_pages"));
  sym_pages_fill_cdf = ID2SYM(rb_intern("heap_pages_fill_cdf"));
}
