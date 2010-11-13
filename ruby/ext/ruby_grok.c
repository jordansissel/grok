#include <ruby.h>
#include <grok.h>
#include "rgrok.h"
#include "ruby_grokmatch.h"

VALUE cGrok; /* Grok class object */

/*
 * Document-class: Grok
 * Grok ruby bindings.
 *
 * For an intro to grok, see:
 * See http://code.google.com/p/semicomplete/wiki/Grok
 * and http://code.google.com/p/semicomplete/wiki/GrokRuby
 *
 * General usage of grok is as follows:
 *
 *    grok = Grok.new
 *    grok.add_patterns_from_file("/path/to/patterns")
 *    grok.add_pattern("WORD", "\b\w+\b")
 *    grok.compile("%{WORD} world!")
 *    m = grok.match("Hello world!")
 *    if m
 *      puts m.captures["WORD"]
 *     else
 *       puts "No match."
 *    fi
 *
 * Grok autodiscovery:
 *    grok = Grok.new
 *    grok.add_patterns_from_file("/path/to/patterns")
 *
 *    # Assuming you loaded the default pattern set
 *    grok.discover("2009-04-18")   
 *    #=> "\\Q\\E%{DATE_EU}\\Q\\E"
 *    grok.discover("Visit http://www.google.com/")   
 *    #=> "\\QVisit \\E%{URI}\\Q\\E"
 *
 */

extern VALUE cGrokMatch;
extern VALUE cGrokDiscover;
extern void Init_GrokMatch();
extern void Init_GrokDiscover();

static VALUE rGrok_initialize(VALUE self) {
  /* empty */
  return Qnil;
}

void rGrok_free(void *p) {
  grok_t *grok = (grok_t *)p;

  /* we strdup our pattern from ruby and rb_str2cstr */
  /* typecast to ignore warnings about freeing a const...
   * TODO(sissel): Fix the constness */
  free((char *)grok->pattern);

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

VALUE rGrok_new_from_grok(grok_t *grok) {
  VALUE rgrok;
  rgrok = Data_Wrap_Struct(cGrok, 0, rGrok_free, grok);
  rb_obj_call_init(rgrok, 0, 0);
  return rgrok;
}

/*
 * Compiles an expression.
 *
 * call-seq:
 *  grok.compile(pattern) 
 *
 */
VALUE rGrok_compile(VALUE self, VALUE pattern) {
  grok_t *grok;
  char *c_pattern = NULL;
  char *str = NULL;
  long len = 0;
  int ret = 0;
  Data_Get_Struct(self, grok_t, grok);

  /* Need strdup here in case 'pattern' object is deleted later in 
   * the ruby code 
   * TODO(sissel): Really, just mark that we are using this string
   * rather than strduping. */
  str = rb_str2cstr(pattern, &len);
  c_pattern = malloc(len);
  memcpy(c_pattern, str, len);

  if (grok->pattern != NULL) {
    /* if we've already called compile, let's free up the string we
     * allocated last time */

    /* typecast to break constness. TODO(sissel): fix const */
    free((char *)grok->pattern);
  }

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

  /* Don't need strdup here, since grok_pattern_add will store a copy
   * if the data */
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

  /* don't need strdup here, since we don't store 'path' long-term */
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
  expanded_pattern = rb_str_new(grok->full_pattern, grok->full_pattern_len);
  return expanded_pattern;
}

VALUE rGrok_pattern(VALUE self) {
  grok_t *grok = NULL;
  VALUE pattern;

  Data_Get_Struct(self, grok_t, grok);
  pattern = rb_str_new(grok->pattern, grok->pattern_len);
  return pattern;
}

VALUE rGrok_patterns(VALUE self) {
  VALUE patternmap = rb_hash_new();
  TCLIST *names = NULL;
  grok_t *grok = NULL;
  int i = 0, len = 0;

  Data_Get_Struct(self, grok_t, grok);
  names = grok_pattern_name_list(grok);

  len = tclistnum(names);
  for (i = 0; i < len; i++)  {
    int namelen = 0;
    const char *name = tclistval(names, i, &namelen);
    size_t regexplen = 0;
    const char *regexp = NULL;
    grok_pattern_find(grok, name, namelen, &regexp, &regexplen);

    VALUE key;
    VALUE value;
    key = rb_tainted_str_new(name, namelen);
    value = rb_tainted_str_new(regexp, regexplen);
    rb_hash_aset(patternmap, key, value);
  }
  tclistdel(names);
  return patternmap;
}

VALUE rGrok_get_logmask(VALUE self) {
  grok_t *grok;
  Data_Get_Struct(self, grok_t, grok);
  return INT2FIX(grok->logmask);
}

VALUE rGrok_set_logmask(VALUE self, VALUE mask) {
  grok_t *grok;
  Data_Get_Struct(self, grok_t, grok);
  grok->logmask = FIX2INT(mask);
  return Qnil;
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
  rb_define_method(cGrok, "patterns", rGrok_patterns, 0);
  rb_define_method(cGrok, "logmask=", rGrok_set_logmask, 1);
  rb_define_method(cGrok, "logmask", rGrok_get_logmask, 0);

  Init_GrokMatch();
  Init_GrokDiscover();
}
