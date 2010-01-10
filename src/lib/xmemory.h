

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
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


