

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
    { "Ipause", EVENT_CONTROL_PAUSE,          EVENT_FIELD_NONE, NULL, false },
    { "Iresume", EVENT_CONTROL_RESUME,        EVENT_FIELD_NONE, NULL, false },
    { "Ipattern", EVENT_CONTROL_PLAY_PATTERN, EVENT_FIELD_INT, v_pattern, false },
    { "Ireceive", EVENT_CONTROL_RECEIVE_EVENT, EVENT_FIELD_STRING, NULL, false },

    { "I.Bn", EVENT_CONTROL_ENV_SET_BOOL_NAME, EVENT_FIELD_STRING, v_key, false },
    { "I.B",  EVENT_CONTROL_ENV_SET_BOOL,     EVENT_FIELD_BOOL, v_any_bool, false },
    { "I.In", EVENT_CONTROL_ENV_SET_INT_NAME, EVENT_FIELD_STRING, v_key, false },
    { "I.I",  EVENT_CONTROL_ENV_SET_INT,      EVENT_FIELD_INT, v_any_int, false },
    { "I.Fn", EVENT_CONTROL_ENV_SET_FLOAT_NAME, EVENT_FIELD_STRING, v_key, false },
    { "I.F",  EVENT_CONTROL_ENV_SET_FLOAT,    EVENT_FIELD_DOUBLE, v_any_float, false },
    { "I.Tn", EVENT_CONTROL_ENV_SET_TIMESTAMP_NAME, EVENT_FIELD_STRING, v_key, false },
    { "I.T",  EVENT_CONTROL_ENV_SET_TIMESTAMP, EVENT_FIELD_RELTIME, v_any_ts, false },

    { "I.gr", EVENT_CONTROL_SET_GOTO_ROW,     EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "I.gs", EVENT_CONTROL_SET_GOTO_SECTION, EVENT_FIELD_INT, v_section, false },
    { "I.gss", EVENT_CONTROL_SET_GOTO_SUBSONG, EVENT_FIELD_INT, v_subsong, false },
    { "Ig",   EVENT_CONTROL_GOTO,             EVENT_FIELD_NONE, NULL, false },

    { "I.infinite", EVENT_CONTROL_INFINITE,    EVENT_FIELD_BOOL, v_any_bool, false },

    { "#",     EVENT_GENERAL_COMMENT,         EVENT_FIELD_STRING, v_any_str, false },

    { "?",    EVENT_GENERAL_COND,            EVENT_FIELD_BOOL, v_any_bool, false },
    { "?if",   EVENT_GENERAL_IF,              EVENT_FIELD_NONE, NULL, false },
    { "?else", EVENT_GENERAL_ELSE,            EVENT_FIELD_NONE, NULL, false },
    { "?end", EVENT_GENERAL_END_IF,         EVENT_FIELD_NONE, NULL, false },

    { "signal", EVENT_GENERAL_SIGNAL,        EVENT_FIELD_STRING, v_any_str, false },
    { "callBn", EVENT_GENERAL_CALL_BOOL_NAME, EVENT_FIELD_STRING, v_any_str, false },
    { "callB", EVENT_GENERAL_CALL_BOOL,      EVENT_FIELD_BOOL, v_any_bool, false },
    { "callIn", EVENT_GENERAL_CALL_INT_NAME, EVENT_FIELD_STRING, v_any_str, false },
    { "callI", EVENT_GENERAL_CALL_INT,       EVENT_FIELD_INT, v_any_int, false },
    { "callFn", EVENT_GENERAL_CALL_FLOAT_NAME, EVENT_FIELD_STRING, v_any_str, false },
    { "callF", EVENT_GENERAL_CALL_FLOAT,     EVENT_FIELD_DOUBLE, v_any_float, false },

    { "mpd",   EVENT_GLOBAL_PATTERN_DELAY,    EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "m.jc",  EVENT_GLOBAL_SET_JUMP_COUNTER, EVENT_FIELD_INT, v_counter, false },
    { "m.jr",  EVENT_GLOBAL_SET_JUMP_ROW,     EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "m.js",  EVENT_GLOBAL_SET_JUMP_SECTION, EVENT_FIELD_INT, v_section, false },
    { "m.jss", EVENT_GLOBAL_SET_JUMP_SUBSONG, EVENT_FIELD_INT, v_subsong, false },

    { "m.s",   EVENT_GLOBAL_SET_SCALE,        EVENT_FIELD_INT, v_scale, false },
    { "m.so",  EVENT_GLOBAL_SET_SCALE_OFFSET, EVENT_FIELD_DOUBLE, v_finite_float, false },
    { "mms",   EVENT_GLOBAL_MIMIC_SCALE,      EVENT_FIELD_INT, v_scale, false },
    { "m.sfp", EVENT_GLOBAL_SET_SCALE_FIXED_POINT, EVENT_FIELD_INT, v_note_entry, false },
    { "mssi",  EVENT_GLOBAL_SHIFT_SCALE_INTERVALS, EVENT_FIELD_INT, v_note_entry, false },

    { "m.t",   EVENT_GLOBAL_SET_TEMPO,        EVENT_FIELD_DOUBLE, v_tempo, false },
    { "m/t",   EVENT_GLOBAL_SLIDE_TEMPO,      EVENT_FIELD_DOUBLE, v_tempo, false },
    { "m/=t",  EVENT_GLOBAL_SLIDE_TEMPO_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "m.v",   EVENT_GLOBAL_SET_VOLUME,       EVENT_FIELD_DOUBLE, v_volume, false },
    { "m/v",   EVENT_GLOBAL_SLIDE_VOLUME,     EVENT_FIELD_DOUBLE, v_volume, false },
    { "m/=v",  EVENT_GLOBAL_SLIDE_VOLUME_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { ".i",   EVENT_CHANNEL_SET_INSTRUMENT,  EVENT_FIELD_INT, v_ins, false },
    { ".g",   EVENT_CHANNEL_SET_GENERATOR,   EVENT_FIELD_INT, v_gen, false },
    { ".e",   EVENT_CHANNEL_SET_EFFECT,      EVENT_FIELD_INT, v_effect, false },
    { ".ge",  EVENT_CHANNEL_SET_GLOBAL_EFFECTS, EVENT_FIELD_NONE, NULL, false },
    { ".ie",  EVENT_CHANNEL_SET_INSTRUMENT_EFFECTS, EVENT_FIELD_NONE, NULL, false },
    { ".d",   EVENT_CHANNEL_SET_DSP,         EVENT_FIELD_INT, v_dsp, false },

    { "n+",   EVENT_CHANNEL_NOTE_ON,         EVENT_FIELD_DOUBLE, v_pitch, false },
    { "h",    EVENT_CHANNEL_HIT,             EVENT_FIELD_INT, v_hit, false },
    { "n-",   EVENT_CHANNEL_NOTE_OFF,        EVENT_FIELD_NONE, NULL, false },

    { ".f",   EVENT_CHANNEL_SET_FORCE,       EVENT_FIELD_DOUBLE, v_force, false },
    { "/f",   EVENT_CHANNEL_SLIDE_FORCE,     EVENT_FIELD_DOUBLE, v_force, false },
    { "/=f",  EVENT_CHANNEL_SLIDE_FORCE_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "ts",   EVENT_CHANNEL_TREMOLO_SPEED,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "td",   EVENT_CHANNEL_TREMOLO_DEPTH,   EVENT_FIELD_DOUBLE, v_tremolo_depth, false },
    { "tdd",  EVENT_CHANNEL_TREMOLO_DELAY,   EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { "/p",   EVENT_CHANNEL_SLIDE_PITCH,     EVENT_FIELD_DOUBLE, v_pitch, false },
    { "/=p",  EVENT_CHANNEL_SLIDE_PITCH_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "vs",   EVENT_CHANNEL_VIBRATO_SPEED,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "vd",   EVENT_CHANNEL_VIBRATO_DEPTH,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "vdd",  EVENT_CHANNEL_VIBRATO_DELAY,   EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { "<arp", EVENT_CHANNEL_RESET_ARPEGGIO,  EVENT_FIELD_NONE, NULL, false },
    { ".arpn", EVENT_CHANNEL_SET_ARPEGGIO_NOTE, EVENT_FIELD_DOUBLE, v_pitch, false },
    { ".arpi", EVENT_CHANNEL_SET_ARPEGGIO_INDEX, EVENT_FIELD_INT, v_arp_index, false },
    { ".arps", EVENT_CHANNEL_SET_ARPEGGIO_SPEED, EVENT_FIELD_DOUBLE, v_arp_speed, false },
    { "arp+", EVENT_CHANNEL_ARPEGGIO_ON,     EVENT_FIELD_NONE, NULL, false },
    { "arp-", EVENT_CHANNEL_ARPEGGIO_OFF,    EVENT_FIELD_NONE, NULL, false },

    { ".l",   EVENT_CHANNEL_SET_LOWPASS,     EVENT_FIELD_DOUBLE, v_lowpass, false },
    { "/l",   EVENT_CHANNEL_SLIDE_LOWPASS,   EVENT_FIELD_DOUBLE, v_lowpass, false },
    { "/=l",  EVENT_CHANNEL_SLIDE_LOWPASS_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },
    { "ws",   EVENT_CHANNEL_AUTOWAH_SPEED,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "wd",   EVENT_CHANNEL_AUTOWAH_DEPTH,   EVENT_FIELD_DOUBLE, v_nonneg_float, false },
    { "wdd",  EVENT_CHANNEL_AUTOWAH_DELAY,   EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { ".r",   EVENT_CHANNEL_SET_RESONANCE,   EVENT_FIELD_DOUBLE, v_resonance, false },

    { ".P",   EVENT_CHANNEL_SET_PANNING,     EVENT_FIELD_DOUBLE, v_panning, false },
    { "/P",   EVENT_CHANNEL_SLIDE_PANNING,   EVENT_FIELD_DOUBLE, v_panning, false },
    { "/=P",  EVENT_CHANNEL_SLIDE_PANNING_LENGTH, EVENT_FIELD_RELTIME, v_nonneg_ts, false },

    { ".gBn", EVENT_CHANNEL_SET_GEN_BOOL_NAME, EVENT_FIELD_STRING, v_key, false },
    { ".gB",  EVENT_CHANNEL_SET_GEN_BOOL,    EVENT_FIELD_BOOL, v_any_bool, false },
    { ".gIn", EVENT_CHANNEL_SET_GEN_INT_NAME, EVENT_FIELD_STRING, v_key, false },
    { ".gI",  EVENT_CHANNEL_SET_GEN_INT,     EVENT_FIELD_INT, v_any_int, false },
    { ".gFn", EVENT_CHANNEL_SET_GEN_FLOAT_NAME, EVENT_FIELD_STRING, v_key, false },
    { ".gF",  EVENT_CHANNEL_SET_GEN_FLOAT,   EVENT_FIELD_DOUBLE, v_any_float, false },
    { ".gTn", EVENT_CHANNEL_SET_GEN_RELTIME_NAME, EVENT_FIELD_STRING, v_key, false },
    { ".gT",  EVENT_CHANNEL_SET_GEN_RELTIME, EVENT_FIELD_RELTIME, v_any_ts, false },

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

    { "qlocation", EVENT_QUERY_LOCATION,      EVENT_FIELD_NONE, NULL, false },
    { "qvoices", EVENT_QUERY_VOICE_COUNT,     EVENT_FIELD_NONE, NULL, false },
    { "qf",    EVENT_QUERY_ACTUAL_FORCE,      EVENT_FIELD_INT, v_gen, false },

    { "Asubsong", EVENT_AUTO_LOCATION_SUBSONG, EVENT_FIELD_INT, v_subsong, false },
    { "Asection", EVENT_AUTO_LOCATION_SECTION, EVENT_FIELD_INT, v_section, false },
    { "Apattern", EVENT_AUTO_LOCATION_PATTERN, EVENT_FIELD_INT, v_pattern, false },
    { "Arow",  EVENT_AUTO_LOCATION_ROW,       EVENT_FIELD_RELTIME, v_any_ts, false },
    { "Avoices", EVENT_AUTO_VOICE_COUNT,      EVENT_FIELD_INT, v_any_int, false },
    { "Af",    EVENT_AUTO_ACTUAL_FORCE,       EVENT_FIELD_DOUBLE, v_any_float, false },

    { "", EVENT_FIELD_NONE, EVENT_FIELD_NONE, NULL, false }
};


struct Event_names
{
    AAtree* names;
    bool error;
};


static int event_name_cmp(const char* e1, const char* e2);


Event_names* new_Event_names(void)
{
    Event_names* names = xalloc(Event_names);
    if (names == NULL)
    {
        return NULL;
    }
    names->error = false;
    names->names = new_AAtree((int (*)(const void*, const void*))event_name_cmp,
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


static int event_name_cmp(const char* e1, const char* e2)
{
    assert(e1 != NULL);
    assert(e2 != NULL);
    int i = 0;
    for (i = 0; e1[i] != '\0' && e1[i] != '"' &&
                e2[i] != '\0' && e1[i] != '"'; ++i)
    {
        if (e1[i] < e2[i])
        {
            return -1;
        }
        else if (e1[i] > e2[i])
        {
            return 1;
        }
    }
    if (e2[i] != '\0' && e2[i] != '"')
    {
        return -1;
    }
    if (e1[i] != '\0' && e1[i] != '"')
    {
        return 1;
    }
    return 0;
}


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
        if (string_eq(name, "mj"))
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


