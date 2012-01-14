

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
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
#include <string.h>

#include <AAtree.h>
#include <Event.h>
#include <Event_names.h>
#include <Event_type.h>
#include <Param_validator.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


typedef struct Name_info
{
    char name[EVENT_NAME_MAX + 1];
    Event_type type;
    Event_field_type param_type;
    Param_validator validator;
    bool pass;
} Name_info;


static void del_Name_info(Name_info* ni);


static void del_Name_info(Name_info* ni)
{
    (void)ni;
    return;
}


static Name_info event_specs[] =
{
    { ">pause", EVENT_CONTROL_PAUSE,          EVENT_FIELD_NONE, NULL, false },
    { ">resume", EVENT_CONTROL_RESUME,        EVENT_FIELD_NONE, NULL, false },
    { ">pattern", EVENT_CONTROL_PLAY_PATTERN, EVENT_FIELD_INT, v_pattern, false },

    { ">.Bn", EVENT_CONTROL_ENV_SET_BOOL_NAME, EVENT_FIELD_STRING, v_key, false },
    { ">.B",  EVENT_CONTROL_ENV_SET_BOOL,     EVENT_FIELD_BOOL, v_any_bool, false },
    { ">.In", EVENT_CONTROL_ENV_SET_INT_NAME, EVENT_FIELD_STRING, v_key, false },
    { ">.I",  EVENT_CONTROL_ENV_SET_INT,      EVENT_FIELD_INT, v_any_int, false },
    { ">.Fn", EVENT_CONTROL_ENV_SET_FLOAT_NAME, EVENT_FIELD_STRING, v_key, false },
    { ">.F",  EVENT_CONTROL_ENV_SET_FLOAT,    EVENT_FIELD_DOUBLE, v_any_float, false },
    { ">.Tn", EVENT_CONTROL_ENV_SET_TIMESTAMP_NAME, EVENT_FIELD_STRING, v_key, false },
    { ">.T",  EVENT_CONTROL_ENV_SET_TIMESTAMP, EVENT_FIELD_RELTIME, v_any_ts, false },

    { ">.gr", EVENT_CONTROL_SET_GOTO_ROW,     EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { ">.gs", EVENT_CONTROL_SET_GOTO_SECTION, EVENT_FIELD_INT, v_section, false },
    { ">.gss", EVENT_CONTROL_SET_GOTO_SUBSONG, EVENT_FIELD_INT, v_subsong, false },
    { ">g",   EVENT_CONTROL_GOTO,             EVENT_FIELD_NONE, NULL, false },

    { ">Turing", EVENT_CONTROL_TURING,        EVENT_FIELD_BOOL, v_any_bool, false },

    { "#",     EVENT_GENERAL_COMMENT,         EVENT_FIELD_STRING, v_any_str, false },

    { "#?",    EVENT_GENERAL_COND,            EVENT_FIELD_STRING, v_cond, false },
    { "#if",   EVENT_GENERAL_IF,              EVENT_FIELD_BOOL, v_any_bool, false },
    { "#endif", EVENT_GENERAL_END_IF,         EVENT_FIELD_NONE, NULL, false },

    { "#signal", EVENT_GENERAL_SIGNAL,        EVENT_FIELD_STRING, v_any_str, false },
    { "#callBn", EVENT_GENERAL_CALL_BOOL_NAME, EVENT_FIELD_STRING, v_any_str, false },
    { "#callB", EVENT_GENERAL_CALL_BOOL,      EVENT_FIELD_BOOL, v_any_bool, false },
    { "#callIn", EVENT_GENERAL_CALL_INT_NAME, EVENT_FIELD_STRING, v_any_str, false },
    { "#callI", EVENT_GENERAL_CALL_INT,       EVENT_FIELD_INT, v_any_int, false },
    { "#callFn", EVENT_GENERAL_CALL_FLOAT_NAME, EVENT_FIELD_STRING, v_any_str, false },
    { "#callF", EVENT_GENERAL_CALL_FLOAT,     EVENT_FIELD_DOUBLE, v_any_float, false },

    { "wpd",   EVENT_GLOBAL_PATTERN_DELAY,    EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "w.jc",  EVENT_GLOBAL_SET_JUMP_COUNTER, EVENT_FIELD_INT, v_counter, false },
    { "w.jr",  EVENT_GLOBAL_SET_JUMP_ROW,     EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "w.js",  EVENT_GLOBAL_SET_JUMP_SECTION, EVENT_FIELD_INT, v_section, false },
    { "w.jss", EVENT_GLOBAL_SET_JUMP_SUBSONG, EVENT_FIELD_INT, v_subsong, false },

    { "w.s",   EVENT_GLOBAL_SET_SCALE,        EVENT_FIELD_INT, v_scale, false },
    { "w.so",  EVENT_GLOBAL_SET_SCALE_OFFSET, EVENT_FIELD_DOUBLE, v_finite_float, false },
    { "wms",   EVENT_GLOBAL_MIMIC_SCALE,      EVENT_FIELD_INT, v_scale, false },
    { "w.sfp", EVENT_GLOBAL_SET_SCALE_FIXED_POINT, EVENT_FIELD_INT, v_note_entry, false },
    { "wssi",  EVENT_GLOBAL_SHIFT_SCALE_INTERVALS, EVENT_FIELD_INT, v_note_entry, false },

    { "w.t",   EVENT_GLOBAL_SET_TEMPO,        EVENT_FIELD_DOUBLE, v_tempo, false },
    { "w/t",   EVENT_GLOBAL_SLIDE_TEMPO,      EVENT_FIELD_DOUBLE, v_tempo, false },
    { "w/=t",  EVENT_GLOBAL_SLIDE_TEMPO_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "w.v",   EVENT_GLOBAL_SET_VOLUME,       EVENT_FIELD_DOUBLE, v_volume, false },
    { "w/v",   EVENT_GLOBAL_SLIDE_VOLUME,     EVENT_FIELD_DOUBLE, v_volume, false },
    { "w/=v",  EVENT_GLOBAL_SLIDE_VOLUME_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { "c.i",   EVENT_CHANNEL_SET_INSTRUMENT,  EVENT_FIELD_INT, v_ins, false },
    { "c.g",   EVENT_CHANNEL_SET_GENERATOR,   EVENT_FIELD_INT, v_gen, false },
    { "c.e",   EVENT_CHANNEL_SET_EFFECT,      EVENT_FIELD_INT, v_effect, false },
    { "c.ge",  EVENT_CHANNEL_SET_GLOBAL_EFFECTS, EVENT_FIELD_NONE, NULL, false },
    { "c.ie",  EVENT_CHANNEL_SET_INSTRUMENT_EFFECTS, EVENT_FIELD_NONE, NULL, false },
    { "c.d",   EVENT_CHANNEL_SET_DSP,         EVENT_FIELD_INT, v_dsp, false },

    { "cn+",   EVENT_CHANNEL_NOTE_ON,         EVENT_FIELD_DOUBLE, v_pitch, false },
    { "ch",    EVENT_CHANNEL_HIT,             EVENT_FIELD_INT, v_hit, false },
    { "cn-",   EVENT_CHANNEL_NOTE_OFF,        EVENT_FIELD_NONE, NULL, false },

    { "c.f",   EVENT_CHANNEL_SET_FORCE,       EVENT_FIELD_DOUBLE, v_force, false },
    { "c/f",   EVENT_CHANNEL_SLIDE_FORCE,     EVENT_FIELD_DOUBLE, v_force, false },
    { "c/=f",  EVENT_CHANNEL_SLIDE_FORCE_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "cTs",   EVENT_CHANNEL_TREMOLO_SPEED,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "cTd",   EVENT_CHANNEL_TREMOLO_DEPTH,   EVENT_FIELD_DOUBLE, v_tremolo_depth, false },
    { "cTdd",  EVENT_CHANNEL_TREMOLO_DELAY,   EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { "c/p",   EVENT_CHANNEL_SLIDE_PITCH,     EVENT_FIELD_DOUBLE, v_pitch, false },
    { "c/=p",  EVENT_CHANNEL_SLIDE_PITCH_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "cVs",   EVENT_CHANNEL_VIBRATO_SPEED,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "cVd",   EVENT_CHANNEL_VIBRATO_DEPTH,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "cVdd",  EVENT_CHANNEL_VIBRATO_DELAY,   EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { "c<Arp", EVENT_CHANNEL_RESET_ARPEGGIO,  EVENT_FIELD_NONE, NULL, false },
    { "c.Arpn", EVENT_CHANNEL_SET_ARPEGGIO_NOTE, EVENT_FIELD_DOUBLE, v_pitch, false },
    { "c.Arpi", EVENT_CHANNEL_SET_ARPEGGIO_INDEX, EVENT_FIELD_INT, v_arp_index, false },
    { "c.Arps", EVENT_CHANNEL_SET_ARPEGGIO_SPEED, EVENT_FIELD_DOUBLE, v_arp_speed, false },
    { "cArp+", EVENT_CHANNEL_ARPEGGIO_ON,     EVENT_FIELD_NONE, NULL, false },
    { "cArp-", EVENT_CHANNEL_ARPEGGIO_OFF,    EVENT_FIELD_NONE, NULL, false },

    { "c.l",   EVENT_CHANNEL_SET_LOWPASS,     EVENT_FIELD_DOUBLE, v_lowpass, false },
    { "c/l",   EVENT_CHANNEL_SLIDE_LOWPASS,   EVENT_FIELD_DOUBLE, v_lowpass, false },
    { "c/=l",  EVENT_CHANNEL_SLIDE_LOWPASS_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "cAs",   EVENT_CHANNEL_AUTOWAH_SPEED,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "cAd",   EVENT_CHANNEL_AUTOWAH_DEPTH,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "cAdd",  EVENT_CHANNEL_AUTOWAH_DELAY,   EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { "c.r",   EVENT_CHANNEL_SET_RESONANCE,   EVENT_FIELD_DOUBLE, v_resonance, false },

    { "c.P",   EVENT_CHANNEL_SET_PANNING,     EVENT_FIELD_DOUBLE, v_panning, false },
    { "c/P",   EVENT_CHANNEL_SLIDE_PANNING,   EVENT_FIELD_DOUBLE, v_panning, false },
    { "c/=P",  EVENT_CHANNEL_SLIDE_PANNING_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { "c.gBn", EVENT_CHANNEL_SET_GEN_BOOL_NAME, EVENT_FIELD_STRING, v_key, false },
    { "c.gB",  EVENT_CHANNEL_SET_GEN_BOOL,    EVENT_FIELD_BOOL, v_any_bool, false },
    { "c.gIn", EVENT_CHANNEL_SET_GEN_INT_NAME, EVENT_FIELD_STRING, v_key, false },
    { "c.gI",  EVENT_CHANNEL_SET_GEN_INT,     EVENT_FIELD_INT, v_any_int, false },
    { "c.gFn", EVENT_CHANNEL_SET_GEN_FLOAT_NAME, EVENT_FIELD_STRING, v_key, false },
    { "c.gF",  EVENT_CHANNEL_SET_GEN_FLOAT,   EVENT_FIELD_DOUBLE, v_any_float, false },
    { "c.gTn", EVENT_CHANNEL_SET_GEN_RELTIME_NAME, EVENT_FIELD_STRING, v_key, false },
    { "c.gT",  EVENT_CHANNEL_SET_GEN_RELTIME, EVENT_FIELD_RELTIME, v_any_ts, false },

    { "i.sus", EVENT_INS_SET_SUSTAIN,         EVENT_FIELD_DOUBLE, v_sustain, false },

    { "g.Bn",  EVENT_GENERATOR_SET_BOOL_NAME, EVENT_FIELD_STRING, v_key, false },
    { "g.B",   EVENT_GENERATOR_SET_BOOL,      EVENT_FIELD_BOOL, v_any_bool, false },
    { "g.In",  EVENT_GENERATOR_SET_INT_NAME,  EVENT_FIELD_STRING, v_key, false },
    { "g.I",   EVENT_GENERATOR_SET_INT,       EVENT_FIELD_INT, v_any_int, false },
    { "g.Fn",  EVENT_GENERATOR_SET_FLOAT_NAME, EVENT_FIELD_STRING, v_key, false },
    { "g.F",   EVENT_GENERATOR_SET_FLOAT,     EVENT_FIELD_DOUBLE, v_any_float, false },
    { "g.Tn",  EVENT_GENERATOR_SET_RELTIME_NAME, EVENT_FIELD_STRING, v_key, false },
    { "g.T",   EVENT_GENERATOR_SET_RELTIME,   EVENT_FIELD_RELTIME, v_any_ts, false },

    { "ebp+",  EVENT_EFFECT_BYPASS_ON,        EVENT_FIELD_NONE, NULL, false },
    { "ebp-",  EVENT_EFFECT_BYPASS_OFF,       EVENT_FIELD_NONE, NULL, false },

    { "d.Bn",  EVENT_DSP_SET_BOOL_NAME,       EVENT_FIELD_STRING, v_key, false },
    { "d.B",   EVENT_DSP_SET_BOOL,            EVENT_FIELD_BOOL, v_any_bool, false },
    { "d.In",  EVENT_DSP_SET_INT_NAME,        EVENT_FIELD_STRING, v_key, false },
    { "d.I",   EVENT_DSP_SET_INT,             EVENT_FIELD_INT, v_any_int, false },
    { "d.Fn",  EVENT_DSP_SET_FLOAT_NAME,      EVENT_FIELD_STRING, v_key, false },
    { "d.F",   EVENT_DSP_SET_FLOAT,           EVENT_FIELD_DOUBLE, v_any_float, false },
    { "d.Tn",  EVENT_DSP_SET_RELTIME_NAME,    EVENT_FIELD_STRING, v_key, false },
    { "d.T",   EVENT_DSP_SET_RELTIME,         EVENT_FIELD_RELTIME, v_any_ts, false },

    { "", EVENT_FIELD_NONE, EVENT_FIELD_NONE, NULL, false }
};


struct Event_names
{
    AAtree* names;
    bool error;
};


Event_names* new_Event_names(void)
{
    Event_names* names = xalloc(Event_names);
    if (names == NULL)
    {
        return NULL;
    }
    names->error = false;
    names->names = new_AAtree((int (*)(const void*, const void*))strcmp,
                              (void (*)(void*))del_Name_info);
    if (names->names == NULL)
    {
        del_Event_names(names);
        return NULL;
    }
    for (int i = 0; event_specs[i].name[0] != '\0'; ++i)
    {
        assert(strlen(event_specs[i].name) > 0);
        assert(strlen(event_specs[i].name) < EVENT_NAME_MAX);
        assert(!AAtree_contains(names->names, event_specs[i].name));
        if (!AAtree_ins(names->names, &event_specs[i]))
        {
            del_Event_names(names);
            return NULL;
        }
    }
    return names;
}


#if 0
bool Event_names_add(Event_names* names, const char* name, Event_type type)
{
    assert(names != NULL);
    assert(name != NULL);
    assert(strlen(name) > 0);
    assert(strlen(name) < EVENT_NAME_MAX);
    assert(!AAtree_contains(names->names, name));
    assert(Event_type_is_supported(type));
    if (names->error)
    {
        return false;
    }
    Name_info* info = xalloc(Name_info);
    if (info == NULL)
    {
        names->error = true;
        return false;
    }
    strncpy(info->name, name, EVENT_NAME_MAX);
    info->name[EVENT_NAME_MAX] = '\0';
    info->type = type;
    info->pass = false;
    if (!AAtree_ins(names->names, info))
    {
        xfree(info);
        names->error = true;
        return false;
    }
    return true;
}
#endif


bool Event_names_error(Event_names* names)
{
    assert(names != NULL);
    return names->error;
}


Event_type Event_names_get(Event_names* names, const char* name)
{
    assert(names != NULL);
    assert(name != NULL);
    Name_info* info = AAtree_get_exact(names->names, name);
    if (info == NULL)
    {
        if (string_eq(name, "wj"))
        {
            return EVENT_GLOBAL_JUMP;
        }
        return EVENT_NONE;
    }
    return info->type;
}


Event_field_type Event_names_get_param_type(Event_names* names,
                                            const char* name)
{
    assert(names != NULL);
    assert(name != NULL);
    Name_info* info = AAtree_get_exact(names->names, name);
    assert(info != NULL);
    return info->param_type;
}


bool Event_names_set_pass(Event_names* names, const char* name, bool pass)
{
    assert(names != NULL);
    assert(name != NULL);
    Name_info* info = AAtree_get_exact(names->names, name);
    if (info == NULL)
    {
        return false;
    }
    info->pass = pass;
    return true;
}


bool Event_names_get_pass(Event_names* names, const char* name)
{
    assert(names != NULL);
    assert(name != NULL);
    Name_info* info = AAtree_get_exact(names->names, name);
    if (info == NULL)
    {
        return false;
    }
    return info->pass;
}


void del_Event_names(Event_names* names)
{
    if (names == NULL)
    {
        return;
    }
    del_AAtree(names->names);
    xfree(names);
    return;
}


