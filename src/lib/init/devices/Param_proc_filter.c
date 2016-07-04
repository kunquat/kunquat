

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


#include <init/devices/Param_proc_filter.h>

#include <containers/Vector.h>
#include <debug/assert.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <string/common.h>
#include <string/Streader.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


// Type of values stored in exclusion lists
typedef int16_t Xindex;


struct Param_proc_filter
{
    Vector* excluded;
};


static bool read_excluded(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    ignore(index);
    rassert(userdata != NULL);

    Param_proc_filter* pf = userdata;

    // Get processor index
    char proc_id[16] = "";
    if (!Streader_read_string(sr, 16, proc_id))
        return false;

    const Xindex proc_index = (Xindex)string_extract_index(proc_id, "proc_", 2, "");
    if ((proc_index < 0) || (proc_index >= KQT_PROCESSORS_MAX))
    {
        Streader_set_error(sr, "Invalid processor ID: %s", proc_id);
        return false;
    }

    if (!Vector_append(pf->excluded, &proc_index))
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for parameter processor filter");
        return false;
    }

    return true;
}


Param_proc_filter* new_Param_proc_filter(Streader* sr)
{
    rassert(sr != NULL);

    if (Streader_is_error_set(sr))
        return NULL;

    Param_proc_filter* pf = memory_alloc_item(Param_proc_filter);
    if (pf == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for parameter processor filter");
        return NULL;
    }

    pf->excluded = new_Vector(sizeof(Xindex));
    if (pf->excluded == NULL)
    {
        del_Param_proc_filter(pf);
        Streader_set_memory_error(
                sr, "Could not allocate memory for parameter processor filter");
        return NULL;
    }

    if (!Streader_read_list(sr, read_excluded, pf))
    {
        rassert(Streader_is_error_set(sr));
        del_Param_proc_filter(pf);
        return NULL;
    }

    return pf;
}


bool Param_proc_filter_is_proc_allowed(const Param_proc_filter* pf, int proc_index)
{
    rassert(pf != NULL);
    rassert(proc_index >= 0);
    rassert(proc_index < KQT_PROCESSORS_MAX);

    const int64_t length = Vector_size(pf->excluded);
    for (int64_t i = 0; i < length; ++i)
    {
        Xindex* excluded = Vector_get_ref(pf->excluded, i);
        if (*excluded == proc_index)
            return false;
    }

    return true;
}


void del_Param_proc_filter(Param_proc_filter* pf)
{
    if (pf == NULL)
        return;

    del_Vector(pf->excluded);
    memory_free(pf);

    return;
}


