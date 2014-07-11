/* common.h - Common includes and macros
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

#ifndef REDPILE_COMMON_H
#define REDPILE_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define ERROR(MESSAGE) do { fprintf(stderr, MESSAGE); exit(EXIT_FAILURE); } while(0)
#define ERROR_IF(CONDITION, MESSAGE) if (CONDITION) { ERROR(MESSAGE); }
#define CHECK_OOM(POINTER) ERROR_IF(!POINTER, "Out of memory!\n")
#define IS_POWER_OF_TWO(x) ((x & (x - 1)) == 0)

#endif