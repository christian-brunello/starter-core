/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_RULES_H_INCLUDED
#define YY_YY_RULES_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
enum yytokentype
{
  YYEMPTY = -2,
  YYEOF = 0,			/* "end of file"  */
  YYerror = 256,		/* error  */
  YYUNDEF = 257,		/* "invalid token"  */
  T_EQ = 258,			/* T_EQ  */
  T_NE = 259,			/* T_NE  */
  T_LT = 260,			/* T_LT  */
  T_LE = 261,			/* T_LE  */
  T_GT = 262,			/* T_GT  */
  T_GE = 263,			/* T_GE  */
  T_AND = 264,			/* T_AND  */
  T_OR = 265,			/* T_OR  */
  T_KEY_VAR = 266,		/* T_KEY_VAR  */
  T_KEY_SET = 267,		/* T_KEY_SET  */
  T_KEY_WHEN = 268,		/* T_KEY_WHEN  */
  T_KEY_TRIGGER = 269,		/* T_KEY_TRIGGER  */
  VALUE = 270,			/* VALUE  */
  IDENTIFIER = 271,		/* IDENTIFIER  */
  UMINUS = 272			/* UMINUS  */
};
typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define T_EQ 258
#define T_NE 259
#define T_LT 260
#define T_LE 261
#define T_GT 262
#define T_GE 263
#define T_AND 264
#define T_OR 265
#define T_KEY_VAR 266
#define T_KEY_SET 267
#define T_KEY_WHEN 268
#define T_KEY_TRIGGER 269
#define VALUE 270
#define IDENTIFIER 271
#define UMINUS 272

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 38 "rules.y"

  double dval;
  gchar *sval;

  Expr *expr_ptr;
  Assign *ass_ptr;
  Trigger *trigger_ptr;
  LabelEntry *label_entry_ptr;
  LabelBlock *label_block_ptr;
  GPtrArray *ptr_array;

#line 113 "rules.h"

};
typedef union YYSTYPE YYSTYPE;
#define YYSTYPE_IS_TRIVIAL 1
#define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (Engine * engine);


#endif /* !YY_YY_RULES_H_INCLUDED  */
