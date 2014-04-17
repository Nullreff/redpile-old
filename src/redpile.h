/* redpile.h - High performance redstone
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

#ifndef REDPILE_H
#define REDPILE_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define REDPILE_VERSION "0.2.0"

#define ERROR_IF(CONDITION, MESSAGE) if (CONDITION) { fprintf(stderr, MESSAGE); exit(EXIT_FAILURE); }
#define CHECK_OOM(POINTER) ERROR_IF(!POINTER, "Out of memory!\n")

typedef struct {
    int world_size;
    int interactive:1;
    int silent:1;
} RedpileConfig;

#endif
