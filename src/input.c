/* input.c - Input and networking
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
#include "input.h"
#include "linenoise.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int listen_fd, comm_fd;
struct sockaddr_in servaddr;

static int input_read_network(char* buff, int buffsize)
{
    return read(comm_fd, buff, buffsize);
}

static int input_read_linenoise(char* buff, int buffsize)
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

static int input_read_stdin(char* buff, int buffsize)
{
    return read(STDIN_FILENO, buff, buffsize);
}

void input_setup(void)
{
    // Configuration is only required for sockets
    if (config->port == 0)
        return;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(config->port);

    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listen_fd, 10);
    comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
}

int read_input(char *buff, int buffsize)
{
    if (config->port > 0)
        return input_read_network(buff, buffsize);
    else if (config->interactive)
        return input_read_linenoise(buff, buffsize);
    else
        return input_read_stdin(buff, buffsize);
}

