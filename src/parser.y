/* parser.y - Command line instruction parser
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

#define PARSE_ERROR(...) fprintf(stderr, __VA_ARGS__); YYABORT;
#define PARSE_ERROR_IF(CONDITION, ...) if (CONDITION) { fprintf(stderr, __VA_ARGS__); YYABORT; }

int yylex(void);
void yyerror(const char* const message);
bool type_parse(char* string, Type* type);
bool direction_parse(char* string, Direction* dir);
%}

%code requires {
    #include "command.h"
    #include "location.h"
    #include "node.h"
}

%union {
	int integer;
	char *string;
    Location location;
    Type type;
    Direction direction;
    SetArgs set_args;
}

%token LINE_BREAK
%token COMMENT

/* Data */
%token <integer> INT
%token <string> STRING
%type  <location> location
%type  <type> type
%type  <direction> direction
%type  <set_args> set_args
%type  <integer> tick_args

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

location: INT INT INT { $$ = location_create($1, $2, $3); }
;

type: STRING { Type type; if (!type_parse($1, &type)) YYABORT; $$ = type; }
;

direction: STRING { Direction dir; if (!direction_parse($1, &dir)) YYABORT; $$ = dir; }
;

anything: | STRING anything { free($1); }
          | INT anything
;

set_args: /* empty */   { $$ = (SetArgs){0, 0};  }
        | direction     { $$ = (SetArgs){$1, 0}; }
        | direction INT { PARSE_ERROR_IF($2 < 0, "State must be non-negative\n");
                          PARSE_ERROR_IF($2 > 3, "State must be less than three\n");
                          $$ = (SetArgs){$1, $2}; }
;

tick_args: /* empty */ { $$ = 1; }
         | INT         { PARSE_ERROR_IF($1 < 0, "Tick count must be greater than zero\n"); $$ = $1; }
         | STRING      { PARSE_ERROR("Tick count must be numeric\n"); }
;

command: PING                        { command_ping();                }
       | STATUS                      { command_status();              }
       | SET location type set_args  { command_set($2, $3, $4);       }
       | GET location                { command_get($2);               }
       | TICK tick_args              { command_tick($2, LOG_NORMAL);  }
       | VTICK tick_args             { command_tick($2, LOG_VERBOSE); }
       | STICK tick_args             { command_tick($2, LOG_SILENT);  }
       | MESSAGES                    { command_messages();            }
       | COMMENT anything
       | STRING anything             { PARSE_ERROR("Unknown command '%s'\n", $1); }
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
            free(string);
            return true;
        }
    }

    fprintf(stderr, "Unknown type: '%s'\n", string);
    free(string);
    return false;
}

bool direction_parse(char* string, Direction* dir)
{
    for (int i = 0; i < DIRECTIONS_COUNT; i++)
    {
        if (strcasecmp(string, Directions[i]) == 0)
        {
            *dir = i;
            free(string);
            return true;
        }
    }

    fprintf(stderr, "Unknown direction: '%s'\n", string);
    free(string);
    return false;
}

