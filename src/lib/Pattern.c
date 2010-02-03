

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


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>

#include <Pattern.h>
#include <Playdata.h>
#include <Event_queue.h>
#include <Event.h>
#include <Event_global.h>
#include <Event_ins.h>
#include <Event_handler.h>

#include <xmemory.h>


Pattern* new_Pattern(void)
{
    Pattern* pat = xalloc(Pattern);
    if (pat == NULL)
    {
        return NULL;
    }
    pat->global = new_Column(NULL);
    if (pat->global == NULL)
    {
        xfree(pat);
        return NULL;
    }
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        pat->cols[i] = new_Column(NULL);
        if (pat->cols[i] == NULL)
        {
            for (--i; i >= 0; --i)
            {
                del_Column(pat->cols[i]);
            }
            del_Column(pat->global);
            xfree(pat);
            return NULL;
        }
    }
    Reltime_set(&pat->length, 16, 0);
    return pat;
}


bool Pattern_parse_header(Pattern* pat, char* str, Read_state* state)
{
    assert(pat != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    Reltime* len = PATTERN_DEFAULT_LENGTH;
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        str = read_const_string(str, "length", state);
        str = read_const_char(str, ':', state);
        str = read_reltime(str, len, state);
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            return false;
        }
    }
    if (Reltime_get_beats(len) < 0)
    {
        Read_state_set_error(state, "Pattern length is negative");
        return false;
    }
    Pattern_set_length(pat, len);
    return true;
}


void Pattern_set_length(Pattern* pat, Reltime* length)
{
    assert(pat != NULL);
    assert(length != NULL);
    assert(length->beats >= 0);
    Reltime_copy(&pat->length, length);
    return;
}


Reltime* Pattern_get_length(Pattern* pat)
{
    assert(pat != NULL);
    return &pat->length;
}


void Pattern_set_col(Pattern* pat, int index, Column* col)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    assert(col != NULL);
    Column* old_col = pat->cols[index];
    pat->cols[index] = col;
    del_Column(old_col);
    return;
}


Column* Pattern_get_col(Pattern* pat, int index)
{
    assert(pat != NULL);
    assert(index >= 0);
    assert(index < KQT_COLUMNS_MAX);
    return pat->cols[index];
}


void Pattern_set_global(Pattern* pat, Column* col)
{
    assert(pat != NULL);
    assert(col != NULL);
    Column* old_col = pat->global;
    pat->global = col;
    del_Column(old_col);
    return;
}


Column* Pattern_get_global(Pattern* pat)
{
    assert(pat != NULL);
    return pat->global;
}


uint32_t Pattern_mix(Pattern* pat,
                     uint32_t nframes,
                     uint32_t offset,
                     Event_handler* eh)
{
//  assert(pat != NULL);
    assert(offset < nframes);
    assert(eh != NULL);
    Playdata* play = Event_handler_get_global_state(eh);
    uint32_t mixed = offset;
//    fprintf(stderr, "new mixing cycle from %" PRIu32 " to %" PRIu32 "\n", offset, nframes);
    if (pat == NULL)
    {
        assert(!play->silent);
        Reltime* limit = Reltime_fromframes(RELTIME_AUTO,
                nframes - mixed,
                play->tempo,
                play->freq);
        for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
        {
            Channel_set_voices(play->channels[i],
                               play->voice_pool,
                               NULL,
                               &play->pos,
                               limit,
                               nframes,
                               mixed,
                               play->tempo,
                               play->freq);
        }
        uint16_t active_voices = Voice_pool_mix_bg(play->voice_pool,
                nframes, mixed, play->freq, play->tempo);
        if (active_voices == 0)
        {
            play->active_voices = 0;
            play->mode = STOP;
            return nframes;
        }
        if (play->active_voices < active_voices)
        {
            play->active_voices = active_voices;
        }
        return nframes;
    }
    Reltime* zero_time = Reltime_set(RELTIME_AUTO, 0, 0);
    while (mixed < nframes
            // TODO: and we still want to mix this pattern
            && Reltime_cmp(&play->pos, &pat->length) <= 0)
    {
        Column_iter_change_col(play->citer, pat->global);
        Event* next_global = Column_iter_get(play->citer, &play->pos);
        Reltime* next_global_pos = NULL;
        if (next_global != NULL)
        {
            next_global_pos = Event_get_pos(next_global);
        }
        int event_index = 0;
        // - Evaluate global events
        while (next_global != NULL
                && Reltime_cmp(next_global_pos, &play->pos) == 0
                && Reltime_cmp(&play->delay_left, zero_time) <= 0
                && !play->jump)
        {
            // FIXME: conditional event handling must be processed here
            //        instead of Song_mix.
            if (play->delay_event_index >= 0)
            {
                for (int i = 0; i <= play->delay_event_index; ++i)
                {
                    next_global = Column_iter_get_next(play->citer);
                    ++event_index;
                }
                if (next_global != NULL)
                {
                    next_global_pos = Event_get_pos(next_global);
                }
                play->delay_event_index = -1;
                if (next_global == NULL
                        || Reltime_cmp(next_global_pos, &play->pos) != 0)
                {
                    break;
                }
            }
            if (Event_get_type((Event*)next_global) == EVENT_GLOBAL_JUMP)
            {
                // Jump events inside Patterns contain mutable state data, so
                // they need to be handled as a special case here.
                Event_global_process((Event_global*)next_global, play);
            }
            else
            {
                if (!Event_handler_handle(eh, -1,
                                     Event_get_type((Event*)next_global),
                                     Event_get_fields((Event*)next_global)))
                {
                    // An internal Event was skipped due to invalid data
                    assert(false);
                }
            }
            if (next_global->type == EVENT_GLOBAL_PATTERN_DELAY)
            {
                play->delay_event_index = event_index;
            }
            ++event_index;
            next_global = Column_iter_get_next(play->citer);
            if (next_global != NULL)
            {
                next_global_pos = Event_get_pos(next_global);
            }
        }
        if (play->old_tempo != play->tempo || play->old_freq != play->freq)
        {
            if (play->volume_slide != 0)
            {
                double update_dB = log2(play->volume_slide_update) * 6;
                update_dB *= (double)play->old_freq / play->freq;
                update_dB *= play->tempo / play->old_tempo;
                play->volume_slide_update = exp2(update_dB / 6);
                play->volume_slide_frames *= (double)play->freq / play->old_freq;
                play->volume_slide_frames *= play->old_tempo / play->tempo;
            }
            play->old_freq = play->freq;
            play->old_tempo = play->tempo;
        }
        bool delay = Reltime_cmp(&play->delay_left, zero_time) > 0;
        assert(!(delay && play->jump));
        if (!delay && play->jump)
        {
            play->jump = false;
            if (play->mode == PLAY_PATTERN)
            {
                if (play->jump_subsong < 0 && play->jump_section < 0)
                {
                    Reltime_copy(&play->pos, &play->jump_row);
                }
                else
                {
                    Reltime_set(&play->pos, 0, 0);
                }
                break;
            }
            if (play->jump_subsong >= 0)
            {
                play->subsong = play->jump_subsong;
            }
            if (play->jump_section >= 0)
            {
                play->section = play->jump_section;
            }
            Reltime_copy(&play->pos, &play->jump_row);
            break;
        }
        if (!delay && Reltime_cmp(&play->pos, &pat->length) >= 0)
        {
            assert(Reltime_cmp(&play->pos, &pat->length) == 0);
            Reltime_init(&play->pos);
            if (play->mode == PLAY_PATTERN)
            {
                Reltime_set(&play->pos, 0, 0);
                break;
            }
            ++play->section;
            if (play->section >= KQT_SECTIONS_MAX)
            {
                play->section = 0;
                play->pattern = -1;
            }
            else
            {
                play->pattern = KQT_SECTION_NONE;
                Subsong* ss = Subsong_table_get(play->subsongs, play->subsong);
                if (ss != NULL)
                {
                    play->pattern = Subsong_get(ss, play->section);
                }
            }
            break;
        }
        assert(next_global == NULL || next_global_pos != NULL);
        uint32_t to_be_mixed = nframes - mixed;
        if (play->tempo_slide != 0)
        {
            if (Reltime_cmp(&play->tempo_slide_left, zero_time) <= 0)
            {
                play->tempo = play->tempo_slide_target;
                play->tempo_slide = 0;
            }
            else if (Reltime_cmp(&play->tempo_slide_int_left, zero_time) <= 0)
            {
                play->tempo += play->tempo_slide_update;
                if ((play->tempo_slide < 0 && play->tempo < play->tempo_slide_target)
                        || (play->tempo_slide > 0 && play->tempo > play->tempo_slide_target))
                {
                    play->tempo = play->tempo_slide_target;
                    play->tempo_slide = 0;
                }
                else
                {
                    Reltime_set(&play->tempo_slide_int_left, 0, 36756720);
                    if (Reltime_cmp(&play->tempo_slide_int_left, &play->tempo_slide_left) > 0)
                    {
                        Reltime_copy(&play->tempo_slide_int_left, &play->tempo_slide_left);
                    }
                }
            }
        }
        Reltime* limit = Reltime_fromframes(RELTIME_AUTO,
                                            to_be_mixed,
                                            play->tempo,
                                            play->freq);
        if (delay && Reltime_cmp(limit, &play->delay_left) > 0)
        {
            Reltime_copy(limit, &play->delay_left);
            to_be_mixed = Reltime_toframes(limit, play->tempo, play->freq);
        }
        if (play->tempo_slide != 0 && Reltime_cmp(limit, &play->tempo_slide_int_left) > 0)
        {
            Reltime_copy(limit, &play->tempo_slide_int_left);
            to_be_mixed = Reltime_toframes(limit, play->tempo, play->freq);
        }
        Reltime_add(limit, limit, &play->pos);
        // - Check for the end of pattern
        if (!delay && Reltime_cmp(&pat->length, limit) < 0)
        {
            Reltime_copy(limit, &pat->length);
            to_be_mixed = Reltime_toframes(Reltime_sub(RELTIME_AUTO, limit, &play->pos),
                                           play->tempo,
                                           play->freq);
        }
        // - Check first upcoming global event position to figure out how much we can mix for now
        if (!delay && next_global != NULL && Reltime_cmp(next_global_pos, limit) < 0)
        {
            assert(next_global_pos != NULL);
            Reltime_copy(limit, next_global_pos);
            to_be_mixed = Reltime_toframes(Reltime_sub(RELTIME_AUTO, limit, &play->pos),
                                           play->tempo,
                                           play->freq);
        }
        // - Calculate the number of frames to be mixed
        assert(Reltime_cmp(&play->pos, limit) <= 0);
        if (to_be_mixed > nframes - mixed)
        {
            to_be_mixed = nframes - mixed;
        }
        if (!play->silent)
        {
            // - Mix the Voices
/*            uint16_t active_voices = Voice_pool_mix_bg(play->voice_pool,
                    to_be_mixed + mixed, mixed, play->freq, play->tempo); */
            Event* ins_event = NULL;
            uint32_t ins_event_pos = UINT32_MAX;
            uint32_t mix_until = to_be_mixed + mixed;
            uint32_t voices_mixed = mixed;
            bool event_found = Event_queue_get(play->ins_events, &ins_event, &ins_event_pos);
            if (event_found && ins_event_pos < mix_until)
            {
                mix_until = ins_event_pos;
            }
            while (voices_mixed < to_be_mixed + mixed)
            {
                if (Reltime_cmp(&play->delay_left, Reltime_init(RELTIME_AUTO)) <= 0)
                {
                    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
                    {
                        Column_iter_change_col(play->citer, pat->cols[i]);
                        Channel_set_voices(play->channels[i],
                                           play->voice_pool,
                                           play->citer,
                                           &play->pos,
                                           limit,
                                           mix_until,
                                           mixed,
                                           play->tempo,
                                           play->freq);
                    }
                }
                uint16_t active_voices = Voice_pool_mix_bg(play->voice_pool,
                        mix_until, voices_mixed, play->freq, play->tempo);
                if (play->active_voices < active_voices)
                {
                    play->active_voices = active_voices;
                }
                if (event_found)
                {
                    assert(ins_event != NULL);
                    Event_ins_process((Event_ins*)ins_event);
                }
                voices_mixed = mix_until;
                mix_until = to_be_mixed + mixed;
                event_found = Event_queue_get(play->ins_events, &ins_event, &ins_event_pos);
                if (event_found && ins_event_pos < mix_until)
                {
                    mix_until = ins_event_pos;
                }
            }
            while (event_found)
            {
                assert(ins_event != NULL);
                Event_ins_process((Event_ins*)ins_event);
                event_found = Event_queue_get(play->ins_events, &ins_event, &ins_event_pos);
            }
            for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
            {
                Channel_update_state(play->channels[i], to_be_mixed + mixed);
            }
        }
        if ((play->volume != 1 || play->volume_slide != 0))
        {
            if (!play->silent)
            {
                for (uint32_t i = mixed; i < mixed + to_be_mixed; ++i)
                {
                    if (play->volume_slide != 0)
                    {
                        play->volume *= play->volume_slide_update;
                        play->volume_slide_frames -= 1;
                        if (play->volume_slide_frames <= 0)
                        {
                            play->volume = play->volume_slide_target;
                            play->volume_slide = 0;
                        }
                        else if ((play->volume_slide == 1 &&
                                  play->volume > play->volume_slide_target) ||
                                 (play->volume_slide == -1 &&
                                  play->volume < play->volume_slide_target))
                        {
                            play->volume = play->volume_slide_target;
                            play->volume_slide = 0;
                        }
                    }
                    for (int k = 0; k < play->buf_count; ++k)
                    {
                        assert(play->bufs[k] != NULL);
                        play->bufs[k][i] *= play->volume;
                    }
                }
            }
            else if (play->volume_slide != 0)
            {
                play->volume *= pow(play->volume_slide_update, to_be_mixed);
                play->volume_slide_frames -= to_be_mixed;
                if (play->volume_slide_frames <= 0)
                {
                    play->volume = play->volume_slide_target;
                    play->volume_slide = 0;
                }
                else if ((play->volume_slide == 1 &&
                          play->volume > play->volume_slide_target) ||
                         (play->volume_slide == -1 &&
                          play->volume < play->volume_slide_target))
                {
                    play->volume = play->volume_slide_target;
                    play->volume_slide = 0;
                }
            }
        }
        // - Increment play->pos
        Reltime* adv = Reltime_sub(RELTIME_AUTO, limit, &play->pos);
        if (play->tempo_slide != 0)
        {
            Reltime_sub(&play->tempo_slide_int_left, &play->tempo_slide_int_left, adv);
            Reltime_sub(&play->tempo_slide_left, &play->tempo_slide_left, adv);
        }
        Reltime_add(&play->play_time, &play->play_time, adv);
        if (Reltime_cmp(&play->delay_left, Reltime_init(RELTIME_AUTO)) > 0)
        {
            Reltime_sub(&play->delay_left, &play->delay_left, adv);
        }
        else
        {
            Reltime_copy(&play->pos, limit);
        }
        mixed += to_be_mixed;
    }
    return mixed - offset;
}


void del_Pattern(Pattern* pat)
{
    assert(pat != NULL);
    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        del_Column(pat->cols[i]);
    }
    del_Column(pat->global);
    xfree(pat);
    return;
}


