/*
 * starter-core - rules.y
 *
 * Copyright (C) 2026 Christian Brunello <brncrs@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

%{
/* C Decls */
#include <stdio.h>
#include <glib.h>

#include "internals.h"

extern int yylineno;
extern int yylex (void);
void yyerror (Engine *, char const *);

%}

/* Bison decls */
%define parse.error verbose

%parse-param { Engine *engine }

%union {
    double dval; 
    gchar *sval; 

    Expr *expr_ptr;
    Assign *ass_ptr;
    Trigger *trigger_ptr;
    LabelEntry *label_entry_ptr;
    LabelBlock *label_block_ptr;
    GPtrArray *ptr_array; 
}

/* ==, !=, <, <=, >, >=, &&, || */
%token T_EQ T_NE T_LT T_LE T_GT T_GE T_AND T_OR
/* VAR, SET, WHEN, TRIGGER */
%token T_KEY_VAR T_KEY_SET T_KEY_WHEN T_KEY_TRIGGER

%token <dval> VALUE
%token <sval> IDENTIFIER

%type <ptr_array> argument_list
%type <expr_ptr> expr
%type <label_block_ptr> label_block
%type <trigger_ptr> trigger_statement
%type <ptr_array> label_entries
%type <label_entry_ptr> label_entry
%type <ptr_array> assignment_list
%type <ass_ptr> assignment

%left T_AND 
%left T_OR
%left T_EQ T_NE 
%left T_LT T_LE T_GT T_GE
%left '+' '-' 
%left '*' '/'
%right UMINUS

%%

/* Bison rules */

program:
    instruction_list
    ;

instruction_list:
    instruction
    | instruction_list instruction
    ;

instruction:
    label_block
    {
      g_ptr_array_add(engine->labels, $1);
    }
    | trigger_statement
    {
      g_ptr_array_add(engine->triggers, $1);
    }
    | declaration { }
    ;

label_block:
    IDENTIFIER '{' label_entries '}'
    {
      guint i;

      for(i = 0; i < $3->len; i++)
        {
	  LabelEntry *e = $3->pdata[i];

	  g_free(e->ass->id);
	  e->ass->id = g_strdup($1);
        }

      $$ = label_block_new($3);

      free($1);
    }
    ;

label_entries:
    label_entry {
      $$ = g_ptr_array_new_full(0, (GDestroyNotify) label_entry_delete);
      g_ptr_array_add($$, $1);
    }
    | label_entries label_entry {
      g_ptr_array_add($1, $2);
    }
    ;

label_entry:
    T_KEY_SET expr T_KEY_WHEN expr ';'
    {
      $$ = label_entry_new($4, assign_new("", $2));
      expr_unref($2);
      expr_unref($4);
    }
    ;

trigger_statement: T_KEY_TRIGGER T_KEY_WHEN expr T_KEY_SET assignment_list ';' 
		 {
		   $$ = trigger_new($3, $5);
		   expr_unref($3);
		 }
		 ;

assignment_list:
    assignment
    {
      $$ = g_ptr_array_new_full(0, (GDestroyNotify) assign_delete);
      g_ptr_array_add($$, $1);
    }
    | assignment_list ',' assignment 
    {
      g_ptr_array_add($$, $3);  
    }
    ;

assignment: 
	  IDENTIFIER '=' expr
	  {
	    $$ = assign_new($1, $3);
	    expr_unref($3);
	    free($1);
	  }

declaration:
    T_KEY_VAR IDENTIFIER ';' {
      Expr *v = expr_new_literal(0);
      g_hash_table_insert(engine->variables, $2, st_var_new($2, v));
      expr_unref(v);

    }
    | T_KEY_VAR IDENTIFIER '=' expr ';' {
      g_hash_table_insert(engine->variables, $2, st_var_new($2, $4));
      expr_unref($4);
    }
    ;

expr: 
      VALUE {
	$$ = expr_new_literal($1);
      }
      | IDENTIFIER {
	$$ = expr_new_ref($1);
	free($1);
      }
      /* Booleans */
      | expr T_EQ expr {
	$$ = expr_new_op("==", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr T_NE expr {
	$$ = expr_new_op("!=", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr T_LT expr {
	$$ = expr_new_op("<", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr T_LE expr {
	$$ = expr_new_op("<=", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr T_GT expr {
	$$ = expr_new_op(">", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr T_GE expr {
	$$ = expr_new_op(">=", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr T_AND expr {
	$$ = expr_new_op("&&", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr T_OR expr {
	$$ = expr_new_op("||", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      /* Math */
      | expr '+' expr {
	$$ = expr_new_op("+", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr '-' expr {
	$$ = expr_new_op("-", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr '*' expr {
	$$ = expr_new_op("*", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | expr '/' expr {
	$$ = expr_new_op("/", $1, $3);
	expr_unref($1);
	expr_unref($3);
      }
      | '(' expr ')' {
	$$ = $2;
      }
      | '*' expr %prec UMINUS {
        Expr *le = expr_new_literal(-1);
	$$ = expr_new_op("*", le, $2);
	expr_unref(le);
	expr_unref($2);
      }
      | IDENTIFIER '(' argument_list ')' {
        if(strcmp($1, "MIN") == 0)
	  {
	    if($3->len < 1)
	      yyerror(engine, "wrong number of arguments for MIN");

	    $$ = expr_new_func_min($3);
	    g_ptr_array_unref($3);
	  }
        else if(strcmp($1, "MAX") == 0)
	  {
	    if($3->len < 1)
	      yyerror(engine, "wrong number of arguments for MAX");

	    $$ = expr_new_func_max($3);
	    g_ptr_array_unref($3);
	  }
        else if(strcmp($1, "ROUND") == 0)
	  {
	    if($3->len != 1)
	      yyerror(engine, "wrong number of arguments for ROUND");

	    $$ = expr_new_func_round($3);
	    g_ptr_array_unref($3);
	  }
        else if(strcmp($1, "LIMIT") == 0)
	  {
	    if($3->len != 3)
	      yyerror(engine, "wrong number of arguments for LIMIT");

	    $$ = expr_new_func_limit($3);
	    g_ptr_array_unref($3);
	  }
        else if(strcmp($1, "EVAL") == 0)
	  {
	    if($3->len != 1)
	      yyerror(engine, "wrong number of arguments for EVAL");

	    $$ = expr_new_func_eval($3);
	    g_ptr_array_unref($3);
	  }
        else if(strcmp($1, "CHANGED") == 0)
	  {
	    if($3->len < 1)
	      yyerror(engine, "wrong number of arguments for CHANGED");

	    $$ = expr_new_func_changed($3);
	    g_ptr_array_unref($3);
	  }
	else
	  {
	    yyerror(engine, "Unknown function");
	  }

	free($1);
      }
;

argument_list:
    /* empty */
    { }
    | expr {
      $$ = g_ptr_array_new_full(0, (GDestroyNotify) expr_unref);
      g_ptr_array_add($$, $1);
    }
    | argument_list ',' expr {
      g_ptr_array_add($$, $3);
    }
    ;

%% 

/* Additional C code */
void yyerror (Engine *engine, char const *s)
{
  fprintf(stderr, "%s: error parsing rules file at line %d: %s\n", PACKAGE, yylineno, s);
}
