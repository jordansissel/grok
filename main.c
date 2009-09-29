#include "grok.h"
#include "grok_program.h"
#include "grok_config.h"

#include "conf.tab.h"

extern FILE *yyin; /* from conf.lex (flex provides this) */

int main() {
  struct config c;
  grok_collection_t *gcol;
  int i;

  yyin = fopen("grok.conf", "r");
  conf_init(&c);
  i = yyparse(&c);
  if (i != 0) {
    fprintf(stderr, "Parsing error in config file\n");
    return 1;
  }

  gcol = grok_collection_init();
  for (i = 0; i < c.nprograms; i++) {
    grok_collection_add(gcol, &(c.programs[i]));
  }
  grok_collection_loop(gcol);

  return 0;
}
