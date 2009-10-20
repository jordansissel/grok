/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     QUOTEDSTRING = 258,
     INTEGER = 259,
     CONF_DEBUG = 260,
     PROGRAM = 261,
     PROG_FILE = 262,
     PROG_EXEC = 263,
     PROG_MATCH = 264,
     PROG_NOMATCH = 265,
     PROG_LOADPATTERNS = 266,
     FILE_FOLLOW = 267,
     EXEC_RESTARTONFAIL = 268,
     EXEC_MINRESTARTDELAY = 269,
     EXEC_RUNINTERVAL = 270,
     EXEC_READSTDERR = 271,
     MATCH_PATTERN = 272,
     MATCH_REACTION = 273,
     MATCH_SHELL = 274,
     MATCH_FLUSH = 275,
     MATCH_BREAK_IF_MATCH = 276
   };
#endif
/* Tokens.  */
#define QUOTEDSTRING 258
#define INTEGER 259
#define CONF_DEBUG 260
#define PROGRAM 261
#define PROG_FILE 262
#define PROG_EXEC 263
#define PROG_MATCH 264
#define PROG_NOMATCH 265
#define PROG_LOADPATTERNS 266
#define FILE_FOLLOW 267
#define EXEC_RESTARTONFAIL 268
#define EXEC_MINRESTARTDELAY 269
#define EXEC_RUNINTERVAL 270
#define EXEC_READSTDERR 271
#define MATCH_PATTERN 272
#define MATCH_REACTION 273
#define MATCH_SHELL 274
#define MATCH_FLUSH 275
#define MATCH_BREAK_IF_MATCH 276




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 18 "conf.y"
{
  char *str;
  int num;
}
/* Line 1489 of yacc.c.  */
#line 96 "conf.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


