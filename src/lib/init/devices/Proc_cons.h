

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_PROC_CONS_H
#define KQT_PROC_CONS_H


#define PROC_TYPE(name) Device_impl* new_Proc_ ## name(void);
#include <init/devices/Proc_types.h>


#endif // KQT_PROC_CONS_H


