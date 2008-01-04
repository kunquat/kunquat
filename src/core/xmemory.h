

/*
 * Copyright 2008 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_XMEMORY_H
#define K_XMEMORY_H


#include <stdlib.h>


#define xalloc(type)             malloc(sizeof(type))

#define xnalloc(type, n)         malloc(sizeof(type) * (n))
#define xcalloc(type, n)         calloc((n), sizeof(type))
#define xrealloc(type, n, ptr)   realloc(ptr, sizeof(type) * (n))

#define xfree(ptr)               free(ptr)


#endif // K_XMEMORY_H


