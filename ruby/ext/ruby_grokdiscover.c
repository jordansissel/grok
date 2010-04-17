#include "rgrok.h"
#include <grok.h>

VALUE cGrokDiscover;
extern VALUE cGrok;

static void rGrokDiscover_free(void *p);

VALUE rGrokDiscover_new(VALUE klass, VALUE grok) {
  VALUE rgd;
  grok_discover_t *gdt = ALLOC(grok_discover_t); //grok_discover_new();
  rgd = Data_Wrap_Struct(klass, 0, rGrokDiscover_free, gdt);

  VALUE initargs[1] = { grok };
  rb_obj_call_init(rgd, 1, initargs);
  return (VALUE)rgd;
}

static void rGrokDiscover_free(void *p) {
  grok_discover_t *gdt = p;
  grok_discover_free(gdt);
}

VALUE rGrokDiscover_initialize(VALUE self, VALUE rb_grok) {
  grok_discover_t *gdt;
  grok_t *grok;
  Data_Get_Struct(self, grok_discover_t, gdt);
  Data_Get_Struct(rb_grok, grok_t, grok);

  grok_discover_init(gdt, grok);
  return Qnil;
}


VALUE rGrokDiscover_discover(VALUE self, VALUE input) {
  char *cstr_discovery;
  char *cstr_input;
  long unused_input_len;
  int discovery_len;
  grok_discover_t *gdt;
  grok_t *grok;

  Data_Get_Struct(self, grok_discover_t, gdt);
  cstr_input = rb_str2cstr(input, &unused_input_len);
  grok_discover(gdt, cstr_input, &cstr_discovery, &discovery_len);
  return rb_str_new(cstr_discovery, discovery_len);
}

void Init_GrokDiscover() {
  cGrokDiscover = rb_define_class("GrokDiscover", rb_cObject);
  rb_define_singleton_method(cGrokDiscover, "new", rGrokDiscover_new, 1);
  rb_define_method(cGrokDiscover, "initialize", rGrokDiscover_initialize, 1);
  rb_define_method(cGrokDiscover, "discover", rGrokDiscover_discover, 1);
}
