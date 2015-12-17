

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_WAV_H
#define K_WAV_H


#include <devices/param_types/Sample.h>
#include <string/Streader.h>

#include <stdbool.h>


bool Sample_parse_wav(Sample* sample, Streader* sr);


#endif // K_WAV_H


