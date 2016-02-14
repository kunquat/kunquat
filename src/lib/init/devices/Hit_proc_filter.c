

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


#include <init/devices/Hit_proc_filter.h>

#include <containers/Vector.h>
#include <debug/assert.h>
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


struct Hit_proc_filter
{
    Vector* excluded;
};


bool read_excluded(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    ignore(index);
    assert(userdata != NULL);

    Hit_proc_filter* hpf = userdata;

    // Get processor index
    char proc_id[16] = "";
    if (!Streader_read_string(sr, 16, proc_id))
        return false;

    const Xindex proc_index = string_extract_index(proc_id, "proc_", 2, "");
    if (proc_index < 0)
    {
        Streader_set_error(sr, "Invalid processor ID: %s", proc_id);
        return false;
    }

    if (!Vector_append(hpf->excluded, &proc_index))
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for hit processor filter");
        return false;
    }

    return true;
}


Hit_proc_filter* new_Hit_proc_filter(Streader* sr)
{
    assert(sr != NULL);

    Hit_proc_filter* hpf = memory_alloc_item(Hit_proc_filter);
    if (hpf == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for hit processor filter");
        return NULL;
    }

    hpf->excluded = new_Vector(sizeof(Xindex));
    if (hpf->excluded == NULL)
    {
        del_Hit_proc_filter(hpf);
        Streader_set_memory_error(
                sr, "Could not allocate memory for hit processor filter");
        return NULL;
    }

    if (!Streader_read_list(sr, read_excluded, hpf))
    {
        assert(Streader_is_error_set(sr));
        del_Hit_proc_filter(hpf);
        return NULL;
    }

    return hpf;
}


bool Hit_proc_filter_is_proc_allowed(const Hit_proc_filter* hpf, int proc_index)
{
    assert(hpf != NULL);
    assert(proc_index >= 0);
    assert(proc_index < KQT_PROCESSORS_MAX);

    const size_t length = Vector_size(hpf->excluded);
    for (size_t i = 0; i < length; ++i)
    {
        Xindex* excluded = Vector_get_ref(hpf->excluded, i);
        if (*excluded == proc_index)
            return false;
    }

    return true;
}


void del_Hit_proc_filter(Hit_proc_filter* hpf)
{
    if (hpf == NULL)
        return;

    del_Vector(hpf->excluded);
    memory_free(hpf);

    return;
}


