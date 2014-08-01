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
#include "io.h"
#define YY_INPUT(buff, res, buffsize) (res = read_input(buff, buffsize))
#define read_input(buff,buffsize) io_read(buff,buffsize)
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
(?i:delete)     { return DELETE;     }
(?i:get)        { return GET;        }
(?i:tick)       { return TICK;       }
(?i:tickv)      { return TICKV;      }
(?i:tickq)      { return TICKQ;      }
(?i:messages)   { return MESSAGES;   }
:[a-zA-Z0-9]+   { yylval.string  = strdup(yytext + 1);       return VALUE;  }
-?[0-9]+        { yylval.integer = strtol(yytext, NULL, 10); return INT;    }
[a-zA-Z0-9]+    { yylval.string  = strdup(yytext);           return STRING; }
%%
