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
bool type_parse(char* string, Type* type);
%}

%code requires {
    #include "../location.h"
    #include "../node.h"
}

%union {
	int integer;
	char *string;
    Location location;
    Type type;
}

%token LINE_BREAK
%token COMMENT

/* Data */
%token <integer> INT
%token <string> STRING
%type  <location> location
%type  <type> type

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

type: STRING                    { Type type; if (!type_parse($1, &type)) YYABORT; return type; }
;

unknown: | STRING unknown
         | INT unknown
;

command: PING                   { command_ping();                }
       | STATUS                 { command_status();              }
       | SET location type      { command_set($2, $3);           }
       | GET location           { command_get($2);               }
       | TICK INT               { command_tick($2, LOG_NORMAL);  }
       | VTICK INT              { command_tick($2, LOG_VERBOSE); }
       | STICK INT              { command_tick($2, LOG_SILENT);  }
       | MESSAGES               { command_messages();            }
       | COMMENT unknown
       | STRING unknown         { command_unknown($1);           }
;
%%

void yyerror(const char* const message)
{
    command_error(message);
}

bool type_parse(char* string, Type* type)
{
    for (int i = 0; i < MATERIALS_COUNT; i++)
    {
        if (strcasecmp(string, Materials[i]) == 0)
        {
            *type = i;
            return true;
        }
    }

    fprintf(stderr, "Unknown type: '%s'\n", string);
    return false;
}

