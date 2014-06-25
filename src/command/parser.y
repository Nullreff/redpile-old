/* command/parser.y - Command line instruction parser and dispatcher
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

%error-verbose
%{
#include <ctype.h>
#include "parser.h"
#include "command.h"

int yylex(void);
void yyerror(const char* const message);
%}

%union {
	int ival;
	char *sval;
}

%token LINE_BREAK

/* Data */
%token <ival> INT
%token <sval> STRING

/* Commands */
%token PING
%token STATUS
%token SET
%token GET
%token TICK
%token VTICK
%token STICK
%token MESSAGES

%%
input:  | input line
;

line:     LINE_BREAK
        | command LINE_BREAK
;

command: PING                   { command_ping();                }
       | STATUS                 { command_status();              }
       | SET INT INT INT STRING { command_set($2, $3, $4, $5);   }
       | GET INT INT INT        { command_get($2, $3, $4);       }
       | TICK INT               { command_tick($2, LOG_NORMAL);  }
       | VTICK INT              { command_tick($2, LOG_VERBOSE); }
       | STICK INT              { command_tick($2, LOG_SILENT);  }
       | MESSAGES               { command_messages();            }
;
%%

void yyerror(const char* const message)
{
    command_error(message);
}

