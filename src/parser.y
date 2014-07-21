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
    Type* type;
    CommandArgs* args;
}

%token LINE_BREAK
%token COMMENT

/* Data */
%token <integer> INT
%token <string> STRING
%token <string> VALUE
%type  <location> location
%type  <type> type
%type  <args> set_args
%type  <integer> tick_args

/* Commands */
%token PING
%token STATUS
%token SET
%token SETR
%token SETRS
%token DELETE
%token GET
%token TICK
%token TICKV
%token TICKQ
%token MESSAGES

%%
input: /* empty */
     | input line
;

line: LINE_BREAK
    | command LINE_BREAK
;

location: INT INT INT { $$ = location_create($1, $2, $3); }
;

type: STRING { Type* type; if (!type_parse($1, &type)) YYABORT; $$ = type; }
;

anything: /* empty */
        | STRING anything { free($1); }
        | INT anything
;

set_args: /* empty */           { $$ = command_args_allocate(MAX_FIELDS); }
        | set_args STRING VALUE { command_args_append($1, $2, $3); $$ = $1; }

tick_args: /* empty */ { $$ = 1; }
         | INT         { PARSE_ERROR_IF($1 < 0, "Tick count must be greater than zero\n"); $$ = $1; }
         | STRING      { PARSE_ERROR("Tick count must be numeric\n"); }
;

command: PING                                           { command_ping();                            }
       | STATUS                                         { command_status();                          }
       | SET location type set_args                     { command_set($2, $3, $4);                   }
       | SETR location location type set_args           { command_setr($2, $3, $4, $5);              }
       | SETRS location location location type set_args { PARSE_ERROR_IF($4.x <= 0, "x_step must be greater than zero\n");
                                                          PARSE_ERROR_IF($4.y <= 0, "y_step must be greater than zero\n");
                                                          PARSE_ERROR_IF($4.z <= 0, "z_step must be greater than zero\n");
                                                          command_setrs($2, $3, $4, $5, $6);         }
       | DELETE location                                { command_delete($2);                        }
       | GET location                                   { command_get($2);                           }
       | TICK tick_args                                 { command_tick($2, LOG_NORMAL);              }
       | TICKV tick_args                                { command_tick($2, LOG_VERBOSE);             }
       | TICKQ tick_args                                { command_tick($2, LOG_QUIET);               }
       | MESSAGES                                       { command_messages();                        }
       | COMMENT anything
       | STRING anything                                { PARSE_ERROR("Unknown command '%s'\n", $1); }
;
%%

void yyerror(const char* const message)
{
    command_error(message);
}


