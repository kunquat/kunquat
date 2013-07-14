

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

#include <Event_common.h>
#include <Event_global_jump.h>
#include <File_base.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <Module.h>
#include <Pattern_location.h>
#include <xassert.h>


typedef struct Jump_context
{
    Pattern_location location;
    uint32_t play_id;
    int64_t counter;
    Pat_inst_ref piref;
    Tstamp row;
} Jump_context;


static Jump_context* new_Jump_context(Pattern_location* loc);

static Jump_context* new_Jump_context(Pattern_location* loc)
{
    assert(loc != NULL);
    Jump_context* jc = memory_alloc_item(Jump_context);
    if (jc == NULL)
    {
        return NULL;
    }
    jc->location = *loc;
    jc->play_id = 0;
    jc->counter = 0;
    jc->piref.pat = -1;
    jc->piref.inst = -1;
    Tstamp_set(&jc->row, 0, 0);
    return jc;
}


void del_Event_global_jump(Event* event);


Event* new_Event_global_jump(Tstamp* pos)
{
    assert(pos != NULL);
    Event_global_jump* event = memory_alloc_item(Event_global_jump);
    if (event == NULL)
    {
        return NULL;
    }
    Event_init((Event*)event, pos, Trigger_jump);
    event->counters = NULL;
    event->counters_iter = NULL;
    event->parent.destroy = del_Event_global_jump;
    event->counters = new_AAtree(
            (int (*)(const void*, const void*))Pattern_location_cmp,
            memory_free);
    event->counters_iter = new_AAiter(event->counters);
    if (event->counters == NULL || event->counters_iter == NULL)
    {
        del_Event_global_jump(&event->parent);
        return NULL;
    }
    //event->play_id = 0;
    //event->counter = 0;
    //Tstamp_set(&event->row, 0, 0);
    return (Event*)event;
}


void Trigger_global_jump_process(Event* event, Master_params* master_params)
{
    assert(event != NULL);
    assert(event->type == Trigger_jump);
    assert(master_params != NULL);

    Event_global_jump* jump = (Event_global_jump*)event;

    // Find jump context
    Pattern_location* key = PATTERN_LOCATION_AUTO;
    if (master_params->playback_state == PLAYBACK_PATTERN)
    {
        // Use a generic context in pattern mode
        key->song = -1;
        key->piref.pat = -1;
        key->piref.inst = -1;
    }
    else
    {
        key->piref = master_params->cur_pos.piref;
    }

    Jump_context* jc = AAtree_get_exact(jump->counters, key);
    assert(jc != NULL);

    if (jc->play_id != master_params->playback_id)
    {
        // Set new jump target
        jc->play_id = master_params->playback_id;
        jc->counter = master_params->jump_counter;
        jc->piref = master_params->jump_target_piref;
        Tstamp_copy(&jc->row, &master_params->jump_target_row);
    }

    if (jc->counter > 0)
    {
        // Resolve pattern instance
        Pat_inst_ref target_piref;
        target_piref = jc->piref;
        if (target_piref.pat < 0 || target_piref.inst < 0)
            target_piref = master_params->cur_pos.piref;

        if (Module_find_pattern_location(
                    master_params->module,
                    &target_piref,
                    &master_params->cur_pos.track,
                    &master_params->cur_pos.system))
        {
            // Perform jump
            master_params->do_jump = true;
            --jc->counter;
            Tstamp_copy(&master_params->cur_pos.pat_pos, &jc->row);
            master_params->cur_ch = 0;
            master_params->cur_trigger = 0;
        }
        else
        {
            // Pattern instance was removed
            jc->play_id = 0;
            jc->counter = 0;
        }
    }
    else
    {
        // Reset jump so that it may be initialised again
        jc->play_id = 0;
    }

    return;
}


bool Trigger_global_jump_set_locations(
        Event_global_jump* event,
        AAtree* locations,
        AAiter* locations_iter)
{
    assert(event != NULL);
    assert(locations != NULL);
    (void)locations;
    assert(locations_iter != NULL);
    Pattern_location* noloc = PATTERN_LOCATION_AUTO;
    noloc->song = -1;
    noloc->piref.pat = -1;
    noloc->piref.inst = -1;
    Jump_context* context = AAtree_get_exact(event->counters, noloc);
    if (context == NULL)
    {
        context = new_Jump_context(noloc);
        if (context == NULL || !AAtree_ins(event->counters, context))
        {
            memory_free(context);
            return false;
        }
    }
    Pattern_location* loc = AAiter_get_at_least(locations_iter, PATTERN_LOCATION_AUTO);
    while (loc != NULL)
    {
        Jump_context* context = AAtree_get_exact(event->counters, loc);
        if (context == NULL)
        {
            context = new_Jump_context(loc);
            if (context == NULL || !AAtree_ins(event->counters, context))
            {
                memory_free(context);
                return false;
            }
        }
        loc = AAiter_get_next(locations_iter);
    }
    return true;
}


void del_Event_global_jump(Event* event)
{
    if (event == NULL)
    {
        return;
    }
    assert(event->type == Trigger_jump);
    Event_global_jump* jump = (Event_global_jump*)event;
    del_AAtree(jump->counters);
    del_AAiter(jump->counters_iter);
    del_Event_default(event);
    return;
}


