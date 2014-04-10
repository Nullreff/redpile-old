/* common.h - Common macros and data used in testing
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

#define RANGE(var,start,end) int var; for (var = start; var <= end; var++)
#define CUBE_RANGE(start,end)\
    RANGE(x,start,end) {\
    RANGE(y,start,end) {\
    RANGE(z,start,end) {
#define CUBE_RANGE_END }}}

// Minunit testing taken from: http://www.jera.com/techinfo/jtns/jtn002.html
extern int tests_run;
#define MU_ASSERT(message, test) do { if (!(test)) return message; } while (0)
#define MU_RUN_TEST(test) do { char *message = test(); tests_run++; \
                            if (message) return message; } while (0)
