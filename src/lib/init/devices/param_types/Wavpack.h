

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_WAVPACK_H
#define KQT_WAVPACK_H


#include <init/devices/param_types/Sample.h>
#include <string/Streader.h>

#include <stdbool.h>


bool Sample_parse_wavpack(Sample* sample, Streader* sr);


#endif // KQT_WAVPACK_H


