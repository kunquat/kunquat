

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <kunquat/Handle.h>

#include <stdlib.h>


typedef int kqt_Module;


const char* kqtfile_get_error(kqt_Module module);


kqt_Handle kqtfile_load_module(const char* path);


