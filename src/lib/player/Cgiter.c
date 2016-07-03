

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/Cgiter.h>

#include <debug/assert.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


void Cgiter_init(Cgiter* cgiter, const Module* module, int col_index)
{
    assert(cgiter != NULL);
    assert(module != NULL);
    assert(col_index >= 0);
    assert(col_index < KQT_COLUMNS_MAX);

    cgiter->module = module;
    cgiter->col_index = col_index;
    Position_init(&cgiter->pos);
    Column_iter_init(&cgiter->citer);

    cgiter->cur_tr.head = NULL;

    cgiter->row_returned = false;

    cgiter->has_finished = false;
    cgiter->is_pattern_playback_state = false;

    return;
}


static const Pat_inst_ref* find_pat_inst_ref(
        const Module* module, int track, int system)
{
    assert(module != NULL);
    assert(track >= 0);
    assert(track < KQT_TRACKS_MAX);
    assert(system >= 0);
    assert(system < KQT_SYSTEMS_MAX);

    const Track_list* tl = Module_get_track_list(module);
    if (tl != NULL && track < Track_list_get_len(tl))
    {
        const int cur_song = Track_list_get_song_index(tl, track);
        const Order_list* ol = Module_get_order_list(module, cur_song);
        if (ol != NULL && system < Order_list_get_len(ol))
        {
            Pat_inst_ref* piref = Order_list_get_pat_inst_ref(ol, system);
            assert(piref != NULL);
            return piref;
        }
    }
    return NULL;
}


void Cgiter_reset(Cgiter* cgiter, const Position* start_pos)
{
    assert(cgiter != NULL);

    if (Position_is_valid(start_pos))
    {
        // Normal playback
        cgiter->pos = *start_pos;
        const Pat_inst_ref* piref = find_pat_inst_ref(
                cgiter->module,
                cgiter->pos.track,
                cgiter->pos.system);
        if (piref != NULL)
            cgiter->pos.piref = *piref;
        else
            cgiter->pos.track = -1;

        cgiter->is_pattern_playback_state = false;
    }
    else
    {
        // Pattern playback mode
        assert(Position_has_valid_pattern_pos(start_pos));
        cgiter->pos = *start_pos;

        cgiter->is_pattern_playback_state = true;
    }

    cgiter->row_returned = false;

    cgiter->has_finished = false;

#if 0
    fprintf(stderr, "iter pos: %d %d %d %d %d %d\n",
            (int)cgiter->pos.track,
            (int)cgiter->pos.system,
            (int)cgiter->pos.pat_pos.beats,
            (int)cgiter->pos.pat_pos.rem,
            (int)cgiter->pos.piref.pat,
            (int)cgiter->pos.piref.inst);
#endif

    // TODO: Prepare for the first Cgiter_peek call

    return;
}


const Trigger_row* Cgiter_get_trigger_row(Cgiter* cgiter)
{
    assert(cgiter != NULL);

    if (Cgiter_has_finished(cgiter))
        return NULL;

    // Don't return a previously returned row
    if (cgiter->row_returned)
        return NULL;
    cgiter->row_returned = true;

    // Find pattern
    const Pattern* pattern = NULL;
    const Pat_inst_ref* piref = NULL;
    if (cgiter->is_pattern_playback_state)
        piref = &cgiter->pos.piref;
    else
        piref = find_pat_inst_ref(cgiter->module, cgiter->pos.track, cgiter->pos.system);

    if (piref != NULL)
        pattern = Module_get_pattern(cgiter->module, piref);

    if (pattern == NULL)
        return NULL;

    // Store current pattern instance for reference
    cgiter->pos.piref = *piref;

    Column* column = Pattern_get_column(pattern, cgiter->col_index);
    if (column == NULL)
        return NULL;

    Column_iter_change_col(&cgiter->citer, column);
    cgiter->cur_tr.head = Column_iter_get_row(&cgiter->citer, &cgiter->pos.pat_pos);
    if (cgiter->cur_tr.head == NULL)
        return NULL;
    assert(cgiter->cur_tr.head->next != NULL);
    assert(cgiter->cur_tr.head->next->trigger != NULL);
    if (Tstamp_cmp(&cgiter->cur_tr.head->next->trigger->pos, &cgiter->pos.pat_pos) > 0)
        return NULL;

    return &cgiter->cur_tr;
}


void Cgiter_clear_returned_status(Cgiter* cgiter)
{
    assert(cgiter != NULL);
    assert(cgiter->row_returned);

    cgiter->row_returned = false;

    return;
}


bool Cgiter_peek(Cgiter* cgiter, Tstamp* dist)
{
    assert(cgiter != NULL);
    assert(dist != NULL);
    assert(Tstamp_cmp(dist, TSTAMP_AUTO) >= 0);

    if (Cgiter_has_finished(cgiter))
        return false;

    // Find pattern
    const Pattern* pattern = NULL;
    const Pat_inst_ref* piref = NULL;
    if (cgiter->is_pattern_playback_state)
        piref = &cgiter->pos.piref;
    else
        piref = find_pat_inst_ref(cgiter->module, cgiter->pos.track, cgiter->pos.system);

    if (piref != NULL)
        pattern = Module_get_pattern(cgiter->module, piref);

    if (pattern == NULL)
        return false;

    // Check pattern end
    const Tstamp* pat_length = Pattern_get_length(pattern);
    const Tstamp* dist_to_end =
        Tstamp_sub(TSTAMP_AUTO, pat_length, &cgiter->pos.pat_pos);

    if (Tstamp_cmp(dist_to_end, TSTAMP_AUTO) <= 0)
    {
        // We cannot move forwards in playback time
        Tstamp_set(dist, 0, 0);
        return true;
    }

    // Check next trigger row
    Column* column = Pattern_get_column(pattern, cgiter->col_index);
    assert(column != NULL);
    Column_iter_change_col(&cgiter->citer, column);

    const Tstamp* epsilon = Tstamp_set(TSTAMP_AUTO, 0, 1);
    Tstamp* next_pos_min = Tstamp_add(TSTAMP_AUTO, &cgiter->pos.pat_pos, epsilon);
    Trigger_list* row = Column_iter_get_row(&cgiter->citer, next_pos_min);

    if (row != NULL)
    {
        assert(row->next != NULL);
        assert(row->next->trigger != NULL);
        if (Tstamp_cmp(&row->next->trigger->pos, pat_length) <= 0)
        {
            // Trigger row found inside this pattern
            const Tstamp* dist_to_row = Tstamp_sub(
                    TSTAMP_AUTO,
                    &row->next->trigger->pos,
                    &cgiter->pos.pat_pos);
            Tstamp_mina(dist, dist_to_row);
            return true;
        }
    }

    // No trigger row found
    Tstamp_mina(dist, dist_to_end);
    return true;
}


static void Cgiter_go_to_next_system(Cgiter* cgiter)
{
    assert(cgiter != NULL);
    assert(!cgiter->is_pattern_playback_state);

    Tstamp_set(&cgiter->pos.pat_pos, 0, 0);

    ++cgiter->pos.system;
    cgiter->pos.piref.pat = -1;
    const Pat_inst_ref* piref =
        find_pat_inst_ref(cgiter->module, cgiter->pos.track, cgiter->pos.system);
    if (piref != NULL)
        cgiter->pos.piref = *piref;
    else
        cgiter->has_finished = true;

    return;
}


void Cgiter_move(Cgiter* cgiter, const Tstamp* dist)
{
    assert(cgiter != NULL);
    assert(dist != NULL);
    assert(Tstamp_cmp(dist, TSTAMP_AUTO) >= 0);

    if (cgiter->pos.piref.pat < 0)
        return;

    // Find current pattern
    const Pattern* pattern = Module_get_pattern(cgiter->module, &cgiter->pos.piref);
    if (pattern == NULL)
    {
        cgiter->has_finished = true;
        return;
    }

    // Check if we are at the end of a pattern
    const Tstamp* pat_length = Pattern_get_length(pattern);
    if (Tstamp_cmp(&cgiter->pos.pat_pos, pat_length) >= 0)
    {
        // dist must be 0 or the pattern length changed
        if (cgiter->is_pattern_playback_state)
        {
            Tstamp_set(&cgiter->pos.pat_pos, 0, 0);

            // Play zero-length pattern only once to avoid infinite loop
            if (Tstamp_cmp(pat_length, TSTAMP_AUTO) == 0)
                cgiter->has_finished = true;
        }
        else
        {
            Cgiter_go_to_next_system(cgiter);
        }

        cgiter->row_returned = false;
        return;
    }

    // Move forwards
    Tstamp_adda(&cgiter->pos.pat_pos, dist);
    if (Tstamp_cmp(dist, TSTAMP_AUTO) > 0)
        cgiter->row_returned = false;

    return;
}


bool Cgiter_has_finished(const Cgiter* cgiter)
{
    assert(cgiter != NULL);
    return cgiter->has_finished;
}


