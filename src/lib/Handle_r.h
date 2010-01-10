

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat. This work
 * is published from various territories.
 */


#ifndef KQT_HANDLE_R_H
#define KQT_HANDLE_R_H


#include <Handle_private.h>
#include <Entries.h>


typedef struct Handle_r
{
    kqt_Handle handle;
    Entries* entries;
} Handle_r;


#endif // KQT_HANDLE_R_H


