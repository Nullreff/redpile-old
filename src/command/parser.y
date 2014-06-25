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

%code requires {
    #include "../location.h"
}

%union {
	int integer;
	char *string;
    Location location;
}

%token LINE_BREAK

/* Data */
%token <integer> INT
%token <string> STRING
%type  <location> location

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

location: INT INT INT           { location_create($1, $2, $3); }
;

unknown: | STRING unknown
         | INT unknown
;

command: PING                   { command_ping();                }
       | STATUS                 { command_status();              }
       | SET location STRING    { command_set($2, $3);           }
       | GET location           { command_get($2);               }
       | TICK INT               { command_tick($2, LOG_NORMAL);  }
       | VTICK INT              { command_tick($2, LOG_VERBOSE); }
       | STICK INT              { command_tick($2, LOG_SILENT);  }
       | MESSAGES               { command_messages();            }
       | STRING unknown         { command_unknown($1);           }
;
%%

void yyerror(const char* const message)
{
    command_error(message);
}

