

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


#include <init/devices/param_types/Sample.h>

#include <debug/assert.h>
#include <memory.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


Sample* new_Sample(void)
{
    Sample* sample = memory_alloc_item(Sample);
    if (sample == NULL)
        return NULL;

//    Sample_params_init(&sample->params);
//    sample->path = NULL;
//    sample->changed = false;
//    sample->is_lossy = false;
    sample->channels = 1;
    sample->bits = 16;
    sample->is_float = false;
    sample->len = 0;
    sample->data[0] = NULL;
    sample->data[1] = NULL;

    return sample;
}


Sample* new_Sample_from_buffers(float* buffers[], int count, int64_t length)
{
    rassert(buffers != NULL);
    rassert(count >= 1);
    rassert(count <= 2);
    rassert(length > 0);

    Sample* sample = new_Sample();
    if (sample == NULL)
        return NULL;

    sample->channels = count;
    sample->bits = 32;
    sample->is_float = true;
    sample->len = length;
    for (int i = 0; i < count; ++i)
    {
        rassert(buffers[i] != NULL);
        sample->data[i] = buffers[i];
    }

    return sample;
}


void* Sample_get_buffer(Sample* sample, int ch)
{
    rassert(sample != NULL);
    rassert(ch >= 0);
    rassert(ch < sample->channels);

    return sample->data[ch];
}


int64_t Sample_get_len(const Sample* sample)
{
    rassert(sample != NULL);
    return sample->len;
}


void del_Sample(Sample* sample)
{
    if (sample == NULL)
        return;

    memory_free(sample->data[0]);
    memory_free(sample->data[1]);
    memory_free(sample);

    return;
}


