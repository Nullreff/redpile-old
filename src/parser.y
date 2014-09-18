/* parser.y - Command line instruction parser
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redpile nor the names of its contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

%error-verbose
%{
#include <ctype.h>
#include "parser.h"

#define PARSE_ERROR(...) repl_print_error(__VA_ARGS__); YYABORT;
#define PARSE_ERROR_IF(CONDITION, ...) if (CONDITION) { repl_print_error(__VA_ARGS__); YYABORT; }
#define PARSE_ERROR_FREE(VAR, ...) repl_print_error(__VA_ARGS__); free(VAR); YYABORT;

int yylex(void);
void yyerror(const char* const message);
%}

%code requires {
    #include "command.h"
    #include "location.h"
    #include "node.h"
    #include "repl.h"
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
%token NODE
%token NODER
%token NODERS
%token FIELD
%token DELETE
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
         | STRING      { PARSE_ERROR_FREE($1, "Tick count must be numeric\n"); }
;

command: PING                                            { command_ping(); }
       | STATUS                                          { command_status(); }
       | NODE location                                   { command_node_get($2); }
       | NODE location type set_args                     { command_node_set($2, $3, $4); }
       | NODER location location                         { command_noder_get($2, $3); }
       | NODER location location type set_args           { command_noder_set($2, $3, $4, $5); }
       | NODERS location location location               { command_noders_get($2, $3, $4); }
       | NODERS location location location type set_args { command_noders_set($2, $3, $4, $5, $6); }
       | FIELD location STRING                           { command_field_get($2, $3); }
       | FIELD location STRING VALUE                     { command_field_set($2, $3, $4); }
       | DELETE location                                 { command_delete($2); }
       | TICK tick_args                                  { command_tick($2, LOG_NORMAL); }
       | TICKV tick_args                                 { command_tick($2, LOG_VERBOSE); }
       | TICKQ tick_args                                 { command_tick($2, LOG_QUIET); }
       | MESSAGES                                        { command_messages(); }
       | COMMENT anything                                { /* NOOP */ }
       | STRING anything                                 { PARSE_ERROR_FREE($1, "Unknown command '%s'\n", $1); }
;
%%

void yyerror(const char* const message)
{
    command_error(message);
}

