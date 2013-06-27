

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
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>

#include <Channel.h>
#include <Channel_state.h>
#include <Environment.h>
#include <Event.h>
#include <Event_handler.h>
#include <kunquat/limits.h>
#include <math_common.h>
#include <memory.h>
#include <Tstamp.h>
#include <xassert.h>


Channel* new_Channel(Ins_table* insts,
                     int num,
                     Voice_pool* pool,
                     Environment* env,
                     double* tempo,
                     uint32_t* freq)
{
    assert(insts != NULL);
    assert(num >= 0);
    assert(num < KQT_COLUMNS_MAX);
    assert(pool != NULL);
    assert(env != NULL);
    assert(tempo != NULL);
    assert(freq != NULL);
    Channel* ch = memory_alloc_item(Channel);
    if (ch == NULL)
    {
        return NULL;
    }
//    ch->note_off = NULL;
/*    ch->single = (Event*)new_Event_voice_note_on(Tstamp_set(TSTAMP_AUTO, -1, 0));
    if (ch->single == NULL)
    {
        del_Event(ch->note_off);
        memory_free(ch);
        return NULL;
    } */
//    ch->single = NULL;
    if (!Channel_state_init(&ch->init_state, num, env))
    {
        memory_free(ch);
        return NULL;
    }
    ch->init_state.insts = insts;
    ch->init_state.fg_count = 0;
    ch->init_state.pool = pool;
    ch->init_state.tempo = tempo;
    ch->init_state.freq = freq;
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        ch->init_state.fg[i] = NULL;
        ch->init_state.fg_id[i] = 0;
    }
    Channel_state_copy(&ch->cur_state, &ch->init_state);
    return ch;
}


void Channel_mix(Channel* ch,
                 Voice_pool* pool,
                 uint32_t nframes,
                 uint32_t offset,
                 double tempo,
                 uint32_t freq)
{
    assert(ch != NULL);
    assert(pool != NULL);
    assert(offset <= nframes);
    assert(tempo > 0);
    assert(freq > 0);
    if (offset >= nframes)
    {
        return;
    }
    LFO_set_mix_rate(&ch->cur_state.vibrato, freq);
    LFO_set_tempo(&ch->cur_state.vibrato, tempo);
    LFO_set_mix_rate(&ch->cur_state.tremolo, freq);
    LFO_set_tempo(&ch->cur_state.tremolo, tempo);
    Slider_set_mix_rate(&ch->cur_state.panning_slider, freq);
    Slider_set_tempo(&ch->cur_state.panning_slider, tempo);
    LFO_set_mix_rate(&ch->cur_state.autowah, freq);
    LFO_set_tempo(&ch->cur_state.autowah, tempo);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        if (ch->cur_state.fg[i] != NULL)
        {
            ch->cur_state.fg[i] = Voice_pool_get_voice(pool,
                                          ch->cur_state.fg[i],
                                          ch->cur_state.fg_id[i]);
            if (ch->cur_state.fg[i] != NULL)
            {
                assert(ch->cur_state.fg[i]->prio > VOICE_PRIO_INACTIVE);
                Voice_mix(ch->cur_state.fg[i], nframes, offset, freq, tempo);
            }
        }
    }
    if (Slider_in_progress(&ch->cur_state.panning_slider))
    {
        Slider_skip(&ch->cur_state.panning_slider, nframes - offset);
    }
    return;
}


void Channel_update_state(Channel* ch, uint32_t mixed)
{
    assert(ch != NULL);
    (void)ch;
    (void)mixed;
    return;
}


void Channel_set_random_seed(Channel* ch, uint64_t seed)
{
    assert(ch != NULL);
    Channel_state_set_random_seed(&ch->init_state, seed);
    return;
}


void Channel_set_event_cache(Channel* ch, Event_cache* cache)
{
    assert(ch != NULL);
    assert(cache != NULL);
    Channel_state_set_event_cache(&ch->init_state, cache);
    ch->cur_state.event_cache = ch->init_state.event_cache;
    return;
}


void Channel_reset(Channel* ch)
{
    assert(ch != NULL);
    Channel_state_reset(&ch->init_state);
    Channel_state_copy(&ch->cur_state, &ch->init_state);
//    Channel_state_copy(&ch->new_state, &ch->init_state);
    Channel_gen_state_clear(ch->cur_state.cgstate);
    for (int i = 0; i < KQT_GENERATORS_MAX; ++i)
    {
        ch->cur_state.fg[i] = NULL;
        ch->cur_state.fg_id[i] = 0;
    }
    ch->cur_state.fg_count = 0;
    return;
}


void del_Channel(Channel* ch)
{
    if (ch == NULL)
    {
        return;
    }
//    assert(ch->note_off != NULL);
//    assert(ch->single != NULL);
//    del_Event(ch->note_off);
//    del_Event(ch->single);
    Channel_state_uninit(&ch->init_state);
    // cur_state must not be uninitialised -- it merely contains references
    // to dynamic structures in init_state.
    memory_free(ch);
    return;
}


