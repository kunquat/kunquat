

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <transient/Cgiter.h>
#include <xassert.h>


void Cgiter_init(Cgiter* cgiter, const Module* module, int col_index)
{
    assert(cgiter != NULL);
    assert(module != NULL);
    assert(col_index >= 0);
    assert(col_index < KQT_COLUMNS_MAX);

    cgiter->module = module;
    cgiter->col_index = col_index;
    Position_init(&cgiter->pos);
    cgiter->citer = new_Column_iter(NULL); // FIXME: handle alloc failure
    assert(cgiter->citer != NULL);

    cgiter->cur_tr.head = NULL;

    return;
}


static const Pat_inst_ref* find_pat_inst_ref(
        const Module* module,
        int16_t track,
        int16_t system)
{
    assert(module != NULL);
    assert(track >= 0);
    assert(track < KQT_TRACKS_MAX);
    assert(system >= 0);
    assert(system < KQT_SYSTEMS_MAX);

    const Track_list* tl = Module_get_track_list(module);
    if (tl != NULL && track < (int16_t)Track_list_get_len(tl))
    {
        const int16_t cur_song = Track_list_get_song_index(tl, track);
        const Order_list* ol = Module_get_order_list(module, cur_song);
        if (ol != NULL && system < (int16_t)Order_list_get_len(ol))
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
    assert(Position_is_valid(start_pos));

    cgiter->pos = *start_pos;
    const Pat_inst_ref* piref = find_pat_inst_ref(
            cgiter->module,
            cgiter->pos.track,
            cgiter->pos.system);
    if (piref != NULL)
        cgiter->pos.piref = *piref;
    else
        cgiter->pos.track = -1;

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

    // Find pattern
    const Pattern* pattern = NULL;
    const Pat_inst_ref* piref = find_pat_inst_ref(
        cgiter->module,
        cgiter->pos.track,
        cgiter->pos.system);
    if (piref != NULL)
        pattern = Module_get_pattern(cgiter->module, piref);

    if (pattern == NULL)
        return NULL;

    Column* column = Pattern_get_column(pattern, cgiter->col_index);
    if (column == NULL)
        return NULL;

    Column_iter_change_col(cgiter->citer, column);
    cgiter->cur_tr.head = Column_iter_get_row(cgiter->citer, &cgiter->pos.pat_pos);
    if (cgiter->cur_tr.head == NULL)
        return NULL;
    assert(cgiter->cur_tr.head->next != NULL);
    assert(cgiter->cur_tr.head->next->event != NULL);
    if (Tstamp_cmp(&cgiter->cur_tr.head->next->event->pos, &cgiter->pos.pat_pos) > 0)
        return NULL;

    return &cgiter->cur_tr;
}


bool Cgiter_peek(const Cgiter* cgiter, Tstamp* dist)
{
    assert(cgiter != NULL);
    assert(dist != NULL);
    assert(Tstamp_cmp(dist, TSTAMP_AUTO) >= 0);

    if (Cgiter_has_finished(cgiter))
        return false;

    Position pos = cgiter->pos;
    Tstamp* advance = TSTAMP_AUTO;
    Tstamp* remaining = Tstamp_copy(TSTAMP_AUTO, dist);

    while (Tstamp_cmp(advance, dist) < 0)
    {
        // Find pattern
        const Pattern* pattern = NULL;
        const Pat_inst_ref* piref = find_pat_inst_ref(
            cgiter->module,
            pos.track,
            pos.system);
        if (piref != NULL)
            pattern = Module_get_pattern(cgiter->module, piref);

        if (pattern == NULL)
        {
            if (Tstamp_cmp(advance, TSTAMP_AUTO) == 0)
                // We started at the end of data on this call
                return false;

            break;
        }

        const Tstamp* pat_length = Pattern_get_length(pattern);
        const Tstamp* dist_to_end = Tstamp_sub(
                TSTAMP_AUTO,
                pat_length,
                &pos.pat_pos);

        // TODO: find triggers

        if (Tstamp_cmp(remaining, dist_to_end) > 0)
        {
            Tstamp_add(advance, advance, dist_to_end);
            Tstamp_sub(remaining, remaining, dist_to_end);

            // Next system, TODO: playback modes...
            ++pos.system;
        }
        else
        {
            Tstamp_add(advance, advance, remaining);
            Tstamp_set(remaining, 0, 0);
        }
    }

    if (Tstamp_cmp(advance, dist) < 0)
    {
        Tstamp_copy(dist, advance);
        return true;
    }

    return false;
}


static void Cgiter_go_to_next_system(Cgiter* cgiter)
{
    assert(cgiter != NULL);

    Tstamp_set(&cgiter->pos.pat_pos, 0, 0);

    // TODO: make this work
    //if (player->state == PLAYBACK_PATTERN)
    //    return;

    ++cgiter->pos.system;
    cgiter->pos.piref.pat = -1;
    const Pat_inst_ref* piref = find_pat_inst_ref(
            cgiter->module,
            cgiter->pos.track,
            cgiter->pos.system);
    if (piref != NULL)
        cgiter->pos.piref = *piref;
    else
        cgiter->pos.track = -1;

    return;
}


void Cgiter_move(Cgiter* cgiter, const Tstamp* dist)
{
    assert(cgiter != NULL);
    assert(dist != NULL);
    assert(Tstamp_cmp(dist, TSTAMP_AUTO) >= 0);

    Tstamp_add(&cgiter->pos.pat_pos, &cgiter->pos.pat_pos, dist);

    Tstamp* remaining = Tstamp_copy(TSTAMP_AUTO, dist);

    while (Tstamp_cmp(remaining, TSTAMP_AUTO) > 0 &&
            cgiter->pos.track >= 0)
    {
        // Find current pattern
        const Pattern* pattern = Module_get_pattern(
                cgiter->module,
                &cgiter->pos.piref);
        if (pattern == NULL)
        {
            cgiter->pos.track = -1;
            return;
        }

        // See if we went past the pattern end
        const Tstamp* pat_length = Pattern_get_length(pattern);
        Tstamp_sub(remaining, &cgiter->pos.pat_pos, pat_length);
        if (Tstamp_cmp(remaining, TSTAMP_AUTO) > 0)
        {
            Cgiter_go_to_next_system(cgiter);
            Tstamp_copy(&cgiter->pos.pat_pos, remaining);
        }
    }

    return;
}


bool Cgiter_has_finished(const Cgiter* cgiter)
{
    assert(cgiter != NULL);
    return !Position_is_valid(&cgiter->pos);
}


