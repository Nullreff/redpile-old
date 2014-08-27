/* repl.c - Main read/eval/print loop
 *
 * Copyright (C) 2014 Ryan Mendivil <ryan@nullreff.net>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "redpile.h"
#include "repl.h"
#include "linenoise.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdarg.h>

int listen_fd, comm_fd;
struct sockaddr_in servaddr;

int yyparse(void);
int yylex_destroy(void);

#define FORMAT_BUFF_SIZE 1000
char* format_buff;

static int repl_read_network(char* buff, int buffsize)
{
    return read(comm_fd, buff, buffsize);
}

static int repl_read_linenoise(char* buff, int buffsize)
{
    char* line = linenoise("> ");
    if (line == NULL)
        return 0;

    // Linenoise has a buffer size of 4096
    // Flex has a default buffer size of at least 8192 on 32 bit
    int size = strlen(line);
    if (size + 2 > buffsize)
    {
        fprintf(stderr, "Line too long, truncating to %i\n", buffsize);
        size = buffsize - 2;
    }

    // Flex won't generate output until we fill it's buffer
    // Since this is interactive mode, we just zero it out
    // and fill it with whatever we read in.
    memset(buff, '\0', buffsize);
    memcpy(buff, line, size);

    // Linenoise strips out the line return
    buff[size] = '\n';

    free(line);
    return buffsize;
}

static int repl_read_stdin(char* buff, int buffsize)
{
    return read(STDIN_FILENO, buff, buffsize);
}

static void repl_print_network(const char* format, va_list ap)
{
    size_t count = vsnprintf(format_buff, FORMAT_BUFF_SIZE, format, ap);
    size_t written = write(comm_fd, format_buff, count);
    if (written < 0)
        ERROR("Problem writing to network\n");
}

static void repl_print_stdout(const char* format, va_list ap)
{
    vprintf(format, ap);
}

static void repl_print_stderr(const char* format, va_list ap)
{
    vfprintf(stderr, format, ap);
}

void repl_run(void)
{
    format_buff = malloc(sizeof(char) * FORMAT_BUFF_SIZE);

    if (config->port > 0)
    {
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htons(INADDR_ANY);
        servaddr.sin_port = htons(config->port);

        bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
        listen(listen_fd, 10);
        printf("Listening on 0.0.0.0:%d\n", config->port);

        int result;
        while (1)
        {
            comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
            do { result = yyparse(); } while (result != 0);
        };
    }
    else
    {
        int result;
        do { result = yyparse(); } while (result != 0);
    }
}

void repl_cleanup(void)
{
    free(format_buff);
    yylex_destroy();
}

int repl_read(char *buff, int buffsize)
{
    if (config->port > 0)
        return repl_read_network(buff, buffsize);
    else if (config->interactive)
        return repl_read_linenoise(buff, buffsize);
    else
        return repl_read_stdin(buff, buffsize);
}

void repl_print(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    if (config->port > 0)
        repl_print_network(format, ap);
    else
        repl_print_stdout(format, ap);
    va_end(ap);
}

void repl_print_error(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    if (config->port > 0)
        repl_print_network(format, ap);
    else
        repl_print_stderr(format, ap);
    va_end(ap);
}

