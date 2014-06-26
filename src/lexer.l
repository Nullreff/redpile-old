/* lexer.l - Command line instruction lexer
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

%{
#include <stdio.h>
#include "parser.h"
extern int read_input(char *buff, int buffsize);
#define YY_INPUT(buff, res, buffsize) (res = read_input(buff, buffsize))
%}
%option noyywrap
%option nounput
%option noinput
%%
[ \t\0]         ;
\n              { return LINE_BREAK; }
#               { return COMMENT;    }
(?i:ping)       { return PING;       }
(?i:status)     { return STATUS;     }
(?i:set)        { return SET;        }
(?i:setr)       { return SETR;       }
(?i:setrs)      { return SETRS;      }
(?i:get)        { return GET;        }
(?i:tick)       { return TICK;       }
(?i:vtick)      { return VTICK;      }
(?i:stick)      { return STICK;      }
(?i:messages)   { return MESSAGES;   }
-?[0-9]+        { yylval.integer = strtol(yytext, NULL, 10); return INT;    }
[a-zA-Z0-9]+    { yylval.string  = strdup(yytext);           return STRING; }
%%
