

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Voice_group.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <mathnum/common.h>
#include <player/Voice.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


Voice_group* Voice_group_init(
        Voice_group* vg, Voice** voices, int offset, int vp_size)
{
    rassert(vg != NULL);
    rassert(voices != NULL);
    rassert(offset >= 0);
    rassert(offset < vp_size);
    rassert(vp_size > 0);

    vg->voices = voices + offset;

    const uint64_t group_id = Voice_get_group_id(voices[offset]);
    if (group_id == 0)
    {
        vg->size = 0;
        return vg;
    }

    vg->size = 1;

    for (int i = offset + 1; i < vp_size; ++i)
    {
        rassert(voices[i] != NULL);
        if (Voice_get_group_id(voices[i]) != group_id)
            break;
        ++vg->size;
    }

    for (int i = 0; i < vg->size; ++i)
        vg->voices[i]->updated = false;

    return vg;
}


Voice_group* Voice_group_copy(Voice_group* dest, const Voice_group* src)
{
    rassert(dest != NULL);
    rassert(src != NULL);
    rassert(src != dest);

    memcpy(dest, src, sizeof(Voice_group));

    return dest;
}


int Voice_group_get_size(const Voice_group* vg)
{
    rassert(vg != NULL);
    return vg->size;
}


bool Voice_group_is_bg(const Voice_group* vg)
{
    rassert(vg != NULL);

    if (vg->size == 0)
        return false;

    for (int i = 0; i < vg->size; ++i)
    {
        if (vg->voices[i]->prio == VOICE_PRIO_BG)
            return true;
    }

    return false;
}


int Voice_group_get_active_count(const Voice_group* vg)
{
    rassert(vg != NULL);

    int count = 0;
    for (int i = 0; i < vg->size; ++i)
    {
        if (vg->voices[i]->state->active)
            ++count;
    }

    return count;
}


Voice* Voice_group_get_voice(Voice_group* vg, int index)
{
    rassert(vg != NULL);
    rassert(index >= 0);
    rassert(index < Voice_group_get_size(vg));

    return vg->voices[index];
}


Voice* Voice_group_get_voice_by_proc(Voice_group* vg, uint32_t proc_id)
{
    rassert(vg != NULL);

    for (int i = 0; i < vg->size; ++i)
    {
        const Processor* proc = Voice_get_proc(vg->voices[i]);
        if ((proc != NULL) && (Device_get_id((const Device*)proc) == proc_id))
            return vg->voices[i];
    }

    return NULL;
}


int Voice_group_get_ch_num(const Voice_group* vg)
{
    rassert(vg != NULL);
    rassert(vg->size > 0);

    return Voice_get_ch_num(vg->voices[0]);
}


void Voice_group_deactivate_all(Voice_group* vg)
{
    rassert(vg != NULL);

    for (int i = 0; i < vg->size; ++i)
        Voice_reset(vg->voices[i]);

    return;
}


void Voice_group_deactivate_unreachable(Voice_group* vg)
{
    rassert(vg != NULL);

    for (int i = 0; i < vg->size; ++i)
    {
        Voice* voice = vg->voices[i];
        if (!voice->updated || !voice->state->active)
            Voice_reset(voice);
    }

    return;
}


