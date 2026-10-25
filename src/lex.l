/*
 * starter-core - lex.l
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
#include <glib.h>
#include <starter/defs.h>
#include "internals.h"
#include "rules.h"

extern void yyerror(const char *s);
%}

NUMBER      [-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?
IDENTIFIER  [A-Za-z][A-Za-z0-9_:]*

%%

"=="     { return T_EQ; }
"!="     { return T_NE; }
"<"      { return T_LT; }
"<="     { return T_LE; }
">"      { return T_GT; }
">="     { return T_GE; }
"&&"     { return T_AND; }
"||"     { return T_OR; }

"{"      { return '{'; }
"}"      { return '}'; }
"("      { return '('; }
")"      { return ')'; }
","      { return ','; }
"="      { return '='; }
";"      { return ';'; }

"+"      { return '+'; }
"-"      { return '-'; }
"*"      { return '*'; }
"/"      { return '/'; }

"VAR"   { return T_KEY_VAR; }
"SET"   { return T_KEY_SET; }
"WHEN"   { return T_KEY_WHEN; }
"TRIGGER" { return T_KEY_TRIGGER; }
"FLAG"  { return T_KEY_FLAG; }
"USR1" {
	  yylval.uval = ST_IO_FLAG_USR1;
	  return T_FLAG_NAME;
       }
"USR2" {
	  yylval.uval = ST_IO_FLAG_USR2;
	  return T_FLAG_NAME;
       }
"USR3" {
	  yylval.uval = ST_IO_FLAG_USR3;
	  return T_FLAG_NAME;
       }
"USR4" {
	  yylval.uval = ST_IO_FLAG_USR4;
	  return T_FLAG_NAME;
       }
"MANUAL_OVERRIDE" {
	  yylval.uval = ST_IO_FLAG_MANUAL_OVERRIDE;
	  return T_FLAG_NAME;
       }
"NO_HISTORY" {
	  yylval.uval = ST_IO_FLAG_NO_HISTORY;
	  return T_FLAG_NAME;
       }
"TRUE" {
	  yylval.dval = 1;
	  return VALUE;
       } 

"FALSE" {
	  yylval.dval = 0;
	  return VALUE;
       } 

{NUMBER}    {
                yylval.dval = atof(yytext);
                return VALUE;
            }

{IDENTIFIER} {
                 yylval.sval = g_strdup(yytext);
                 return IDENTIFIER;
             }

[ \t]+      { }


"#"([^\n])*\n { yylineno++; }

[\n]	      { yylineno++; }

.           {
                fprintf(stderr, "Invalid character: '%s'\n", yytext);
                yyerror("Invalid character");
            }

%%

int yywrap(void) {
    return 1;
}
