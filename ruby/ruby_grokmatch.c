#include "rgrok.h"
#include <grok.h>

VALUE cGrokMatch;
extern VALUE cGrok;

static ID id_atend;
static ID id_atstart;
static ID id_atcaptures;
static ID id_atsubject;

static void rGrokMatch_free(void *p);

VALUE rGrokMatch_new(VALUE klass) {
  NEWOBJ(rgm, grok_match_t);
  OBJSETUP(rgm, klass, T_OBJECT);

  return (VALUE)rgm;
}

VALUE rGrokMatch_new_from_grok_match(grok_match_t *gm) {
  VALUE rgrokmatch;
  grok_match_t *my_gm = NULL;
  rgrokmatch = Data_Make_Struct(cGrokMatch, grok_match_t, 0,
                                rGrokMatch_free, my_gm);
  memcpy(my_gm, gm, sizeof(grok_match_t));
  rb_obj_call_init(rgrokmatch, 0, 0);
  return rgrokmatch;
}

VALUE rGrokMatch_initialize(VALUE self) {
  grok_match_t *gm;
  VALUE captures;
  
  Data_Get_Struct(self, grok_match_t, gm);

  /* Set the @captures variable to a hash of array of strings */
  captures = rb_hash_new();
  //captures = rb_eval_string("Hash.new { |h,k| h[k] = Array.new }");
  rb_iv_set(self, "@captures", captures);

  return Qtrue;
}

VALUE rGrokMatch_each_capture(VALUE self) {
  char *name;
  const char *data;
  int namelen, datalen;
  grok_match_t *gm;
  VALUE captures;
  ID pred_include;

  Data_Get_Struct(self, grok_match_t, gm);
  captures = rb_iv_get(self, "@captures");
  pred_include = rb_intern("include?");
  grok_match_walk_init(gm);
  while (grok_match_walk_next(gm, &name, &namelen, &data, &datalen) == 0) {
    VALUE key, value;

#ifdef _TRIM_KEY_EXCESS_IN_C_
  /* This section will skip captures of %{FOO} and rename captures of
   * %{FOO:bar} to just 'bar' */
    size_t koff = 0;
    /* there is no 'strcspn' that takes a length, so do it ourselves */
    while (koff < namelen && name[koff] != ':' && name[koff] != '\0') {
      koff++;
    }

    /* Skip captures that aren't named specially */
    if (koff == namelen) {
      //printf("Skipping %.*s\n", namelen, name);
      continue;
    }

    koff++;

    key = rb_str_new(name + koff, namelen - koff);
#endif

    key = rb_str_new(name, namelen);
    value = rb_str_new(data, datalen);

    // Yield [key, value]
    rb_yield(rb_ary_new3(2, key, value));
    //rb_ary_push(ary, value);
  }

  grok_match_walk_end(gm);
  return Qtrue;
}

VALUE rGrokMatch_captures(VALUE self) {
  VALUE captures = rb_iv_get(self, "@captures");
  VALUE len;
  VALUE id_length;

  id_length = rb_intern("length");
  len = rb_funcall(captures, id_length, 0);
  if (FIX2INT(len) > 0) {
    /* Already have computed the captures hash, just return it */
    return captures;
  }

  /* haven't generated captures hash yet, do it now. */
  char *name;
  const char *data;
  int namelen, datalen;
  grok_match_t *gm;
  ID pred_include;

  Data_Get_Struct(self, grok_match_t, gm);
  pred_include = rb_intern("include?");
  grok_match_walk_init(gm);
  while (grok_match_walk_next(gm, &name, &namelen, &data, &datalen) == 0) {
    VALUE key, value;

#ifdef _TRIM_KEY_EXCESS_IN_C_
  /* This section will skip captures of %{FOO} and rename captures of
   * %{FOO:bar} to just 'bar' */
    size_t koff = 0;
    /* there is no 'strcspn' that takes a length, so do it ourselves */
    while (koff < namelen && name[koff] != ':' && name[koff] != '\0') {
      koff++;
    }

    /* Skip captures that aren't named specially */
    if (koff == namelen) {
      //printf("Skipping %.*s\n", namelen, name);
      continue;
    }

    koff++;

    key = rb_str_new(name + koff, namelen - koff);
#endif

    key = rb_str_new(name, namelen);
    value = rb_str_new(data, datalen);

    VALUE caparray = rb_hash_aref(captures, key);
    if (caparray == Qnil) {
      caparray = rb_ary_new();
      rb_hash_aset(captures, key, caparray);
    }

    rb_ary_push(caparray, value);
  }

  grok_match_walk_end(gm);
  return captures;
}

VALUE rGrokMatch_start(VALUE self) {
  grok_match_t *gm;
  Data_Get_Struct(self, grok_match_t, gm);
  return INT2FIX(gm->start);
}

VALUE rGrokMatch_end(VALUE self) {
  grok_match_t *gm;
  Data_Get_Struct(self, grok_match_t, gm);
  VALUE ret = Qnil;
  ret = rb_iv_get(self, "@end");
  if (ret == Qnil) {
    ret = rb_iv_set(self, "@end", INT2FIX(gm->end));
  }
  return INT2FIX(gm->end);
}

VALUE rGrokMatch_subject(VALUE self) {
  grok_match_t *gm;
  Data_Get_Struct(self, grok_match_t, gm);
  return rb_str_new2(gm->subject);
}

void rGrokMatch_free(void *p) {
  /* empty */
}

void Init_GrokMatch() {
  cGrokMatch = rb_define_class("GrokMatch", rb_cObject);
  rb_define_singleton_method(cGrokMatch, "new", rGrokMatch_new, 0);
  rb_define_method(cGrokMatch, "initialize", rGrokMatch_initialize, 0);
  rb_define_method(cGrokMatch, "captures", rGrokMatch_captures, 0);
  rb_define_method(cGrokMatch, "start", rGrokMatch_start, 0);
  rb_define_method(cGrokMatch, "end", rGrokMatch_end, 0);
  rb_define_method(cGrokMatch, "subject", rGrokMatch_subject, 0);
  rb_define_method(cGrokMatch, "each_capture", rGrokMatch_each_capture, 0);
  id_atend = rb_intern("@end");
  id_atstart = rb_intern("@start");
  id_atcaptures = rb_intern("@captures");
}
