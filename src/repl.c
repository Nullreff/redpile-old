/* repl.c - Main read/eval/print loop
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

#include "redpile.h"
#include "repl.h"
#include "linenoise.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdarg.h>
#include <errno.h>

int listen_fd, comm_fd;
struct sockaddr_in servaddr;

int yyparse(void);
int yylex_destroy(void);

#define FORMAT_BUFF_SIZE 1000
char* format_buff;

static int repl_read_network(char* buff, int buffsize)
{
    ssize_t size = read(comm_fd, buff, buffsize);
    if (size == -1)
    {
        WARN("Trouble reading from socket: %s\n", strerror(errno));
        return 0;
    }
    else
    {
        return size;
    }
}

static int repl_read_linenoise(char* buff, int buffsize)
{
    char* line = linenoise("> ");
    if (line == NULL)
        return 0;

    linenoiseHistoryAdd(line);

    // Linenoise has a buffer size of 4096
    // Flex has a default buffer size of at least 8192 on 32 bit
    int size = strlen(line);
    if (size + 2 > buffsize)
    {
        WARN("Line too long, truncating to %i\n", buffsize);
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
    ssize_t size = read(STDIN_FILENO, buff, buffsize);
    if (size == -1)
    {
        WARN("Trouble reading from stdin: %s\n", strerror(errno));
        return 0;
    }
    else
    {
        return size;
    }
}

static void repl_print_network(const char* format, va_list ap)
{
    size_t count = vsnprintf(format_buff, FORMAT_BUFF_SIZE, format, ap);
    ssize_t written = write(comm_fd, format_buff, count);
    WARN_IF(written == -1, "Trouble writing to socket: %s\n", strerror(errno));
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
        ERROR_IF(listen_fd == -1, "Error creating socket: %s\n", strerror(errno));

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = htons(INADDR_ANY);
        servaddr.sin_port = htons(config->port);

        int result;
        result = bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
        ERROR_IF(result == -1, "Error binding to socket: %s\n", strerror(errno));

        result = listen(listen_fd, 10);
        ERROR_IF(result == -1, "Error opening socket: %s\n", strerror(errno));

        printf("Listening on 0.0.0.0:%d\n", config->port);

        while (1)
        {
            comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
            WARN_IF(comm_fd == -1, "Trouble accepting connection: %s\n", strerror(errno));

            do { result = yyparse(); } while (result != 0);
            close(comm_fd);
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
    close(listen_fd);
    close(comm_fd);
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

