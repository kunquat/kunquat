

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


#ifndef KQT_HANDLE_RWC_H
#define KQT_HANDLE_RWC_H


#include <AAtree.h>

#include <Handle_rw.h>


typedef struct Handle_rwc
{
    Handle_rw handle_rw;
    AAtree* changed_files;
} Handle_rwc;


#endif // KQT_HANDLE_RWC_H


