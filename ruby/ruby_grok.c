#include <ruby.h>
#include <grok.h>
#include "rgrok.h"
#include "ruby_grokmatch.h"

VALUE cGrok; /* Grok class object */


extern VALUE cGrokMatch;
extern void Init_GrokMatch();

static VALUE rGrok_initialize(VALUE self) {
  /* empty */
  return Qnil;
}

static void rGrok_free(void *p) {
  grok_t *grok = (grok_t *)p;
  grok_free(grok);
  free(grok);
}

VALUE rGrok_new(VALUE klass) {
  VALUE rgrok;
  grok_t *grok = ALLOC(grok_t);
  grok_init(grok);
  //grok->logmask = ~0;
  rgrok = Data_Wrap_Struct(klass, 0, rGrok_free, grok);
  rb_obj_call_init(rgrok, 0, 0);
  return rgrok;
}

VALUE rGrok_compile(VALUE self, VALUE pattern) {
  grok_t *grok;
  char *c_pattern;
  long len;
  int ret;
  Data_Get_Struct(self, grok_t, grok);
  c_pattern = rb_str2cstr(pattern, &len);
  ret = grok_compilen(grok, c_pattern, (int)len);
  if (ret) {
    rb_raise(rb_eArgError, "Compile failed: %s", grok->errstr);
  }

  return Qnil;
}

VALUE rGrok_match(VALUE self, VALUE input) {
  grok_t *grok = NULL;
  grok_match_t gm;
  char *c_input = NULL;
  long len = 0;
  int ret = 0;

  Data_Get_Struct(self, grok_t, grok);
  c_input = rb_str2cstr(input, &len);
  ret = grok_execn(grok, c_input, (int)len, &gm);

  VALUE rgm = Qnil;
  
  //fprintf(stderr, "%d\n", ret);
  switch (ret) {
    case GROK_ERROR_NOMATCH:
      rgm = Qfalse;
      break;
    case GROK_OK:
      rgm = rGrokMatch_new_from_grok_match(&gm);
      break;
    default:
      rb_raise(rb_eArgError, "Error from grok_execn: %d", ret);
      rgm = Qfalse;
  }

  return rgm;
}

VALUE rGrok_add_pattern(VALUE self, VALUE name, VALUE pattern) {
  grok_t *grok = NULL;
  char *c_name= NULL, *c_pattern = NULL;
  long namelen = 0, patternlen = 0;

  c_name = rb_str2cstr(name, &namelen);
  c_pattern = rb_str2cstr(pattern, &patternlen);
  Data_Get_Struct(self, grok_t, grok);

  grok_pattern_add(grok, c_name, namelen, c_pattern, patternlen);
  return Qnil;
}

VALUE rGrok_add_patterns_from_file(VALUE self, VALUE path) {
  grok_t *grok = NULL;
  int ret = 0;
  char *c_path = NULL;
  long pathlen = 0;

  c_path = rb_str2cstr(path, &pathlen);
  Data_Get_Struct(self, grok_t, grok);

  ret = grok_patterns_import_from_file(grok, c_path);

  if (ret != GROK_OK) {
    rb_raise(rb_eArgError, "Failed to add patterns from file %s", c_path);
  }

  return Qnil;
}

VALUE rGrok_expanded_pattern(VALUE self) {
  grok_t *grok = NULL;
  VALUE expanded_pattern;

  Data_Get_Struct(self, grok_t, grok);
  expanded_pattern = rb_str_new2(grok->full_pattern);
  return expanded_pattern;
}

VALUE rGrok_pattern(VALUE self) {
  grok_t *grok = NULL;
  VALUE pattern;

  Data_Get_Struct(self, grok_t, grok);
  pattern = rb_str_new2(grok->pattern);
  return pattern;
}

void Init_Grok() {
  cGrok = rb_define_class("Grok", rb_cObject);
  rb_define_singleton_method(cGrok, "new", rGrok_new, 0);
  rb_define_method(cGrok, "initialize", rGrok_initialize, 0);
  rb_define_method(cGrok, "compile", rGrok_compile, 1);
  rb_define_method(cGrok, "match", rGrok_match, 1);
  rb_define_method(cGrok, "expanded_pattern", rGrok_expanded_pattern, 0);
  rb_define_method(cGrok, "pattern", rGrok_pattern, 0);
  rb_define_method(cGrok, "add_pattern", rGrok_add_pattern, 2);
  rb_define_method(cGrok, "add_patterns_from_file",
                   rGrok_add_patterns_from_file, 1);

  Init_GrokMatch();
}
