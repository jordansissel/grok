
struct grok_capture {
  int name_len;
  string name<>;
  int subname_len;
  string subname<>;
  int pattern_len;
  string pattern<>;
  int id;
  int pcre_capture_number;

  int predicate_lib_len;
  string predicate_lib<>;
  int predicate_func_name_len;
  string predicate_func_name<>;

  /* predicate functions store personal data here. */
  opaque extra<>;
};
