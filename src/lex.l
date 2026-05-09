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
