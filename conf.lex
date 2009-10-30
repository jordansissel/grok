%{
#include <string.h>
#include "conf.tab.h"
#include "grok_config.h"
#include "stringhelper.h"
%}

%option noyywrap bison-bridge

true true|yes|on|1
false false|no|off|0
number [0-9]+

%x LEX_COMMENT
%x LEX_STRING
%x LEX_ERROR

%%

program { return PROGRAM; }
load-patterns { return PROG_LOADPATTERNS; }

file { return PROG_FILE; }
follow { return FILE_FOLLOW; }

exec { return PROG_EXEC; }
restart-on-failure { return EXEC_RESTARTONFAIL; }
minimum-restart-delay { return EXEC_MINRESTARTDELAY; }
run-interval { return EXEC_RUNINTERVAL; }
read-stderr { return EXEC_READSTDERR; }

match { return PROG_MATCH; }
no-match { return PROG_NOMATCH; }
pattern { return MATCH_PATTERN; }
reaction { return MATCH_REACTION; }
shell { return MATCH_SHELL; }
flush { return MATCH_FLUSH; }
break-if-match { return MATCH_BREAK_IF_MATCH; }

debug { return CONF_DEBUG; }
none { return SHELL_NONE; }
stdout { return SHELL_STDOUT; }

{true} { yylval->num = 1; return INTEGER; }
{false} { yylval->num = 0; return INTEGER; }
{number} { yylval->num = atoi(yytext); return INTEGER; }

"#" BEGIN(LEX_COMMENT);
<LEX_COMMENT>[^\n]* /* ignore comments */ //{ printf("Comment: %s\n", yytext); }
<LEX_COMMENT>\n   { yylineno++; BEGIN(INITIAL); } /* end comment */

\" { BEGIN(LEX_STRING); }
<LEX_STRING>((\\.)+|[^\\\"]+)* { 
  int len, size;
  len = yyleng;
  yylval->str = string_ndup(yytext, len);
  size = len + 1;
  string_unescape(&yylval->str, &len, &size);
  /* XXX: putting a null at the end shouldn't be necessary */
  yylval->str[len] = '\0';
  return QUOTEDSTRING;
}
<LEX_STRING>\" { BEGIN(INITIAL); }


\{ { return '{'; }
\} { return '}'; }
: { return ':'; }


[ \t] { /* ignore whitespace */ }
[\n] { yylineno++; }

. { BEGIN(LEX_ERROR); unput(yytext[0]); }
<LEX_ERROR>[^\n]* {
  fprintf(stderr, "Unexpected input on line %d: '%.*s'\n",
          yylineno, yyleng, yytext); 
  BEGIN(INITIAL);
  return 256; /* 256 == lexer error */
}
%%
