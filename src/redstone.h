/* redstone.h - Redstone logic implementation
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

#ifndef REDPILE_REDSTONE_H
#define REDPILE_REDSTONE_H

#include "message.h"

#define TYPE_METHOD(TYPE)\
    void redstone_ ## TYPE ## _update(World* world, Node* node, Messages* in, Queue* messages, Queue* sets)

#define TYPE_REGISTER(TYPE)\
    case TYPE: redstone_ ## TYPE ## _update(world, node, in, output, sets); break

TYPE_METHOD(EMPTY);
TYPE_METHOD(AIR);
TYPE_METHOD(INSULATOR);
TYPE_METHOD(CONDUCTOR);
TYPE_METHOD(WIRE);
TYPE_METHOD(PISTON);
TYPE_METHOD(COMPARATOR);
TYPE_METHOD(REPEATER);
TYPE_METHOD(TORCH);
TYPE_METHOD(SWITCH);

#endif
