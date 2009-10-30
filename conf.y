%{ 
#include <stdio.h>
#include <string.h>

#include "conf.tab.h"
#include "grok_config.h"
#include "grok_input.h"
#include "grok_matchconf.h"

int yylineno;
void yyerror (YYLTYPE *loc, struct config *conf, char const *s) {
  fprintf (stderr, "Syntax error: %s\n", s);
}

#define DEBUGMASK(val) ((val > 0) ? ~0 : 0)
%}

%union{
  char *str;
  int num;
}

%token <str> QUOTEDSTRING
%token <num> INTEGER
%token CONF_DEBUG "debug"

%token PROGRAM "program"
%token PROG_FILE "file"
%token PROG_EXEC "exec"
%token PROG_MATCH "match"
%token PROG_NOMATCH "no-match"
%token PROG_LOADPATTERNS "load-patterns"

%token FILE_FOLLOW "follow"

%token EXEC_RESTARTONFAIL "restart-on-failure"
%token EXEC_MINRESTARTDELAY "minimum-restart-delay"
%token EXEC_RUNINTERVAL "run-interval"
%token EXEC_READSTDERR "read-stderr"

%token MATCH_PATTERN "pattern"
%token MATCH_REACTION "reaction"
%token MATCH_SHELL "shell"
%token MATCH_FLUSH "flush"
%token MATCH_BREAK_IF_MATCH "break-if-match"

%token SHELL_STDOUT "stdout"
%token SHELL_NONE "none"

%token '{' '}' ';' ':' '\n'

%pure-parser
%parse-param {struct config *conf}
%error-verbose
%locations

%start config

%%

config: config root 
      | root 
      | error { 
        /* Errors are unrecoverable, so let's return nonzero from the parser */
        return 1;
      }

root: root_program
    | "debug" ':' INTEGER { conf->logmask = DEBUGMASK($3); }

root_program: PROGRAM '{' { conf_new_program(conf); }
                program_block 
              '}' 
       
program_block: program_block program_block_statement 
             | program_block_statement

program_block_statement: program_file 
                 | program_exec
                 | program_match
                 | program_nomatch
                 | program_load_patterns
                 | "debug" ':' INTEGER { CURPROGRAM.logmask = DEBUGMASK($3); }

program_load_patterns: "load-patterns" ':' QUOTEDSTRING 
                     { conf_new_patternfile(conf); CURPATTERNFILE = $3; }

program_file: "file" QUOTEDSTRING { conf_new_input_file(conf, $2); }
            program_file_optional_block

program_file_optional_block: /*empty*/ | '{' file_block '}' 

program_exec: "exec" QUOTEDSTRING { conf_new_input_process(conf, $2); } 
            program_exec_optional_block

program_exec_optional_block: /* empty */ | '{' exec_block '}' 

program_match: "match" '{' { conf_new_matchconf(conf); }
                 match_block
               '}' 

program_nomatch: "no-match" '{' 
               { conf_new_matchconf(conf); CURMATCH.is_nomatch = 1;  }
                   match_block
                 '}' 

file_block: file_block file_block_statement
          | file_block_statement
file_block_statement: /*empty*/
          | "follow" ':' INTEGER { CURINPUT.source.file.follow = $3; }
          | "debug" ':' INTEGER { CURINPUT.logmask = DEBUGMASK($3); }

exec_block: exec_block exec_block_statement
          | exec_block_statement
          
exec_block_statement: /* empty */
          | "restart-on-failure" ':'  INTEGER 
             { CURINPUT.source.process.restart_on_death = $3; }
          | "minimum-restart-delay" ':' INTEGER
             { CURINPUT.source.process.min_restart_delay = $3; }
          | "run-interval" ':' INTEGER
             { CURINPUT.source.process.run_interval = $3; }
          | "read-stderr" ':' INTEGER
             { CURINPUT.source.process.read_stderr = $3; }
          | "debug" ':' INTEGER { CURINPUT.logmask = DEBUGMASK($3); }

match_block: match_block match_block_statement
           | match_block_statement

match_block_statement: /* empty */
           | "pattern" ':' QUOTEDSTRING { grok_compile(&CURMATCH.grok, $3); }
           | "reaction" ':' QUOTEDSTRING { CURMATCH.reaction = $3; }
           | "shell" ':' QUOTEDSTRING { CURMATCH.shell = $3; }
           | "shell" ':' "none" { CURMATCH.shell = "none"; }
           | "shell" ':' "stdout" { CURMATCH.shell = "stdout"; }
           | "flush" ':' INTEGER { CURMATCH.flush = $3; }
           | "break-if-match" ':' INTEGER { CURMATCH.break_if_match = $3; }
           | "debug" ':' INTEGER { CURMATCH.grok.logmask = DEBUGMASK($3); }


