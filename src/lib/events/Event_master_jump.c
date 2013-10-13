

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
#include <Event_master_decl.h>
#include <kunquat/limits.h>
#include <xassert.h>


bool Event_master_jump_process(Master_params* master_params, Value* value)
{
    assert(master_params != NULL);
    (void)value;

    if (master_params->jump_counter <= 0)
        return true;

    // Get a new Jump context
    AAnode* handle = Jump_cache_acquire_context(master_params->jump_cache);
    if (handle == NULL)
    {
        fprintf(stderr, "Error: Out of jump contexts!\n");
        return false;
    }
    Jump_context* jc = AAnode_get_data(handle);

    // Init context
    jc->piref = master_params->cur_pos.piref;
    Tstamp_copy(&jc->row, &master_params->cur_pos.pat_pos);
    jc->ch_num = master_params->cur_ch;
    jc->order = master_params->cur_trigger;

    jc->counter = master_params->jump_counter;

    jc->target_piref = master_params->jump_target_piref;
    Tstamp_copy(&jc->target_row, &master_params->jump_target_row);

    // Store new context handle
    Active_jumps_add_context(master_params->active_jumps, handle);

    master_params->do_jump = true;

    return true;

#if 0
    Event_master_jump* jump = (Event_master_jump*)event;

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
#endif
}


