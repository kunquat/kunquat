

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


#ifndef K_FILE_WAVPACK_H
#define K_FILE_WAVPACK_H


#include <stdbool.h>
#include <stdio.h>

#include <Sample.h>
#include <File_base.h>


bool Sample_parse_wavpack(Sample* sample,
                          void* data,
                          long length,
                          Read_state* state);


#endif // K_FILE_WAVPACK_H


