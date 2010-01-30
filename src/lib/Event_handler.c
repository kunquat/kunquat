

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
#include <assert.h>

#include <Event_handler.h>
#include <Event_type.h>
#include <Channel_state.h>
#include <Playdata.h>
#include <kunquat/limits.h>

#include <events/Event_global_pattern_delay.h>

#include <xmemory.h>


struct Event_handler
{
    bool mute; // FIXME: this is just to make the stupid Channel_state_init happy
    Channel_state ch_states[KQT_COLUMNS_MAX];
    Playdata* global_state;
    bool (*ch_process[EVENT_CHANNEL_LAST])(Channel_state* state,
                                           char* fields);
    bool (*global_process[EVENT_GLOBAL_LAST])(Playdata* state,
                                              char* fields);
//    bool (*ins_process[EVENT_INS_LAST])(Ins_state* state, char* fields);
    // TODO: generator and effect process collections
};


Event_handler* new_Event_handler(Playdata* global_state)
{
    Event_handler* eh = xalloc(Event_handler);
    if (eh == NULL)
    {
        return NULL;
    }
    eh->global_state = global_state;
/*    if (eh->global_state == NULL)
    {
        del_Event_handler(eh);
        return NULL;
    } */

    eh->global_process[EVENT_GLOBAL_PATTERN_DELAY] =
            Event_global_pattern_delay_handle;

    for (int i = 0; i < KQT_COLUMNS_MAX; ++i)
    {
        Channel_state_init(&eh->ch_states[i], i, &eh->mute);
    }
    return eh;
}


void Event_handler_set_ch_process(Event_handler* eh,
                                  Event_type type,
                                  bool (*ch_process)(Channel_state*, char*))
{
    assert(eh != NULL);
    assert(EVENT_IS_CHANNEL(type));
    assert(ch_process != NULL);
    eh->ch_process[type] = ch_process;
    return;
}


void Event_handler_set_global_process(Event_handler* eh,
                                      Event_type type,
                                      bool (*global_process)(Playdata*,
                                                             char*))
{
    assert(eh != NULL);
    assert(EVENT_IS_GLOBAL(type));
    assert(global_process != NULL);
    eh->global_process[type] = global_process;
    return;
}


#if 0
void Event_handler_set_ins_process(Event_handler* eh,
                                   Event_type type,
                                   bool (*ins_process)(Ins_state*, char*))
{
    assert(eh != NULL);
    assert(EVENT_IS_INS(type));
    assert(ins_process != NULL);
    eh->ins_process[type] = ins_process;
    return;
}
#endif


bool Event_handler_handle(Event_handler* eh,
                          int ch,
                          Event_type type,
                          char* fields)
{
    assert(eh != NULL);
    assert(ch >= -1);
    assert(ch < KQT_COLUMNS_MAX);
    assert(EVENT_IS_VALID(type));
    if (EVENT_IS_CHANNEL(type))
    {
        assert(ch >= 0);
        if (eh->ch_process[type] == NULL)
        {
            return false;
        }
        return eh->ch_process[type](&eh->ch_states[ch], fields);
    }
    else if (EVENT_IS_GLOBAL(type))
    {
        assert(ch == -1);
        if (eh->global_process[type] == NULL)
        {
            return false;
        }
        return eh->global_process[type](eh->global_state, fields);
    }
    return false;
}


Playdata* Event_handler_get_global_state(Event_handler* eh)
{
    assert(eh != NULL);
    return eh->global_state;
}


void del_Event_handler(Event_handler* eh)
{
    assert(eh != NULL);
    if (eh->global_state != NULL)
    {
//        del_Playdata(eh->global_state); // TODO: enable if Playdata becomes private
    }
    xfree(eh);
    return;
}


