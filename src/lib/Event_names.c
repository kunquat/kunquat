

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
#include <string.h>

#include <AAtree.h>
#include <Event.h>
#include <Event_names.h>
#include <Event_type.h>
#include <memory.h>
#include <Param_validator.h>
#include <string_common.h>
#include <xassert.h>


typedef struct Name_info
{
    char name[EVENT_NAME_MAX + 1];
    Event_type type;
    Value_type param_type;
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
    { "Ipause", Event_control_pause,          VALUE_TYPE_NONE, NULL, false },
    { "Iresume", Event_control_resume,        VALUE_TYPE_NONE, NULL, false },
    { "Ipattern", Event_control_play_pattern, VALUE_TYPE_PAT_INST_REF, v_piref, false },
    { "Ireceive", Event_control_receive_event, VALUE_TYPE_STRING, NULL, false },

    { "I.Bn", Event_control_env_set_bool_name, VALUE_TYPE_STRING, v_key, false },
    { "I.B",  Event_control_env_set_bool,     VALUE_TYPE_BOOL, v_any_bool, false },
    { "I.In", Event_control_env_set_int_name, VALUE_TYPE_STRING, v_key, false },
    { "I.I",  Event_control_env_set_int,      VALUE_TYPE_INT, v_any_int, false },
    { "I.Fn", Event_control_env_set_float_name, VALUE_TYPE_STRING, v_key, false },
    { "I.F",  Event_control_env_set_float,    VALUE_TYPE_FLOAT, v_any_float, false },
    { "I.Tn", Event_control_env_set_tstamp_name, VALUE_TYPE_STRING, v_key, false },
    { "I.T",  Event_control_env_set_tstamp, VALUE_TYPE_TSTAMP, v_any_ts, false },

    { "I.gr", Event_control_set_goto_row,     VALUE_TYPE_TSTAMP, v_nonneg_ts, false },
    { "I.gs", Event_control_set_goto_section, VALUE_TYPE_INT, v_system, false },
    { "I.gss", Event_control_set_goto_song,   VALUE_TYPE_INT, v_subsong, false },
    { "Ig",   Event_control_goto,             VALUE_TYPE_NONE, NULL, false },

    { "I.infinite", Event_control_infinite,   VALUE_TYPE_BOOL, v_any_bool, false },

    { "#",     Event_general_comment,         VALUE_TYPE_STRING, v_any_str, false },

    { "?",     Event_general_cond,            VALUE_TYPE_BOOL, v_any_bool, false },
    { "?if",   Event_general_if,              VALUE_TYPE_NONE, NULL, false },
    { "?else", Event_general_else,            VALUE_TYPE_NONE, NULL, false },
    { "?end",  Event_general_end_if,          VALUE_TYPE_NONE, NULL, false },

    //{ "signal", Event_general_signal,         VALUE_TYPE_STRING, v_any_str, false },
    //{ "callBn", Event_general_call_bool_name, VALUE_TYPE_STRING, v_any_str, false },
    { "callB", Event_general_call_bool,      VALUE_TYPE_BOOL, v_any_bool, false },
    //{ "callIn", Event_general_call_int_name, VALUE_TYPE_STRING, v_any_str, false },
    { "callI", Event_general_call_int,       VALUE_TYPE_INT, v_any_int, false },
    //{ "callFn", Event_general_call_float_name, VALUE_TYPE_STRING, v_any_str, false },
    { "callF", Event_general_call_float,     VALUE_TYPE_FLOAT, v_any_float, false },

    { "mpd",   Event_master_pattern_delay,    VALUE_TYPE_TSTAMP, v_nonneg_ts, false },
    { "m.jc",  Event_master_set_jump_counter, VALUE_TYPE_INT, v_counter, false },
    { "m.jr",  Event_master_set_jump_row,     VALUE_TYPE_TSTAMP, v_nonneg_ts, false },
    { "m.jp",  Event_master_set_jump_pat_inst, VALUE_TYPE_PAT_INST_REF, v_piref, false },
    { "mj",    Event_master_jump,             VALUE_TYPE_NONE, NULL, false },

    { "m.s",   Event_master_set_scale,        VALUE_TYPE_INT, v_scale, false },
    { "m.so",  Event_master_set_scale_offset, VALUE_TYPE_FLOAT, v_finite_float, false },
    { "mms",   Event_master_mimic_scale,      VALUE_TYPE_INT, v_scale, false },
    { "m.sfp", Event_master_set_scale_fixed_point, VALUE_TYPE_INT, v_note_entry, false },
    { "mssi",  Event_master_shift_scale_intervals, VALUE_TYPE_INT, v_note_entry, false },

    { "m.t",   Event_master_set_tempo,        VALUE_TYPE_FLOAT, v_tempo, false },
    { "m/t",   Event_master_slide_tempo,      VALUE_TYPE_FLOAT, v_tempo, false },
    { "m/=t",  Event_master_slide_tempo_length, VALUE_TYPE_TSTAMP, v_nonneg_ts, false },
    { "m.v",   Event_master_set_volume,       VALUE_TYPE_FLOAT, v_volume, false },
    { "m/v",   Event_master_slide_volume,     VALUE_TYPE_FLOAT, v_volume, false },
    { "m/=v",  Event_master_slide_volume_length, VALUE_TYPE_TSTAMP, v_nonneg_ts, false },

    { ".i",   Event_channel_set_instrument,  VALUE_TYPE_INT, v_ins, false },
    { ".g",   Event_channel_set_generator,   VALUE_TYPE_INT, v_gen, false },
    { ".e",   Event_channel_set_effect,      VALUE_TYPE_INT, v_effect, false },
    { ".ge",  Event_channel_set_global_effects, VALUE_TYPE_NONE, NULL, false },
    { ".ie",  Event_channel_set_instrument_effects, VALUE_TYPE_NONE, NULL, false },
    { ".d",   Event_channel_set_dsp,         VALUE_TYPE_INT, v_dsp, false },

    { "n+",   Event_channel_note_on,         VALUE_TYPE_FLOAT, v_pitch, false },
    { "h",    Event_channel_hit,             VALUE_TYPE_INT, v_hit, false },
    { "n-",   Event_channel_note_off,        VALUE_TYPE_NONE, NULL, false },

    { ".f",   Event_channel_set_force,       VALUE_TYPE_FLOAT, v_force, false },
    { "/f",   Event_channel_slide_force,     VALUE_TYPE_FLOAT, v_force, false },
    { "/=f",  Event_channel_slide_force_length, VALUE_TYPE_TSTAMP, v_nonneg_ts, false },
    { "ts",   Event_channel_tremolo_speed,   VALUE_TYPE_FLOAT, v_nonneg_float, false },
    { "td",   Event_channel_tremolo_depth,   VALUE_TYPE_FLOAT, v_tremolo_depth, false },
    { "tdd",  Event_channel_tremolo_delay,   VALUE_TYPE_TSTAMP, v_nonneg_ts, false },

    { "/p",   Event_channel_slide_pitch,     VALUE_TYPE_FLOAT, v_pitch, false },
    { "/=p",  Event_channel_slide_pitch_length, VALUE_TYPE_TSTAMP, v_nonneg_ts, false },
    { "vs",   Event_channel_vibrato_speed,   VALUE_TYPE_FLOAT, v_nonneg_float, false },
    { "vd",   Event_channel_vibrato_depth,   VALUE_TYPE_FLOAT, v_nonneg_float, false },
    { "vdd",  Event_channel_vibrato_delay,   VALUE_TYPE_TSTAMP, v_nonneg_ts, false },

    { "<arp", Event_channel_reset_arpeggio,  VALUE_TYPE_NONE, NULL, false },
    { ".arpn", Event_channel_set_arpeggio_note, VALUE_TYPE_FLOAT, v_pitch, false },
    { ".arpi", Event_channel_set_arpeggio_index, VALUE_TYPE_INT, v_arp_index, false },
    { ".arps", Event_channel_set_arpeggio_speed, VALUE_TYPE_FLOAT, v_arp_speed, false },
    { "arp+", Event_channel_arpeggio_on,     VALUE_TYPE_NONE, NULL, false },
    { "arp-", Event_channel_arpeggio_off,    VALUE_TYPE_NONE, NULL, false },

    { ".l",   Event_channel_set_lowpass,     VALUE_TYPE_FLOAT, v_lowpass, false },
    { "/l",   Event_channel_slide_lowpass,   VALUE_TYPE_FLOAT, v_lowpass, false },
    { "/=l",  Event_channel_slide_lowpass_length, VALUE_TYPE_TSTAMP, v_nonneg_ts, false },
    { "ws",   Event_channel_autowah_speed,   VALUE_TYPE_FLOAT, v_nonneg_float, false },
    { "wd",   Event_channel_autowah_depth,   VALUE_TYPE_FLOAT, v_nonneg_float, false },
    { "wdd",  Event_channel_autowah_delay,   VALUE_TYPE_TSTAMP, v_nonneg_ts, false },

    { ".r",   Event_channel_set_resonance,   VALUE_TYPE_FLOAT, v_resonance, false },

    { ".P",   Event_channel_set_panning,     VALUE_TYPE_FLOAT, v_panning, false },
    { "/P",   Event_channel_slide_panning,   VALUE_TYPE_FLOAT, v_panning, false },
    { "/=P",  Event_channel_slide_panning_length, VALUE_TYPE_TSTAMP, v_nonneg_ts, false },

    { ".gBn", Event_channel_set_gen_bool_name, VALUE_TYPE_STRING, v_key, false },
    { ".gB",  Event_channel_set_gen_bool,    VALUE_TYPE_BOOL, v_any_bool, false },
    { ".gIn", Event_channel_set_gen_int_name, VALUE_TYPE_STRING, v_key, false },
    { ".gI",  Event_channel_set_gen_int,     VALUE_TYPE_INT, v_any_int, false },
    { ".gFn", Event_channel_set_gen_float_name, VALUE_TYPE_STRING, v_key, false },
    { ".gF",  Event_channel_set_gen_float,   VALUE_TYPE_FLOAT, v_any_float, false },
    { ".gTn", Event_channel_set_gen_tstamp_name, VALUE_TYPE_STRING, v_key, false },
    { ".gT",  Event_channel_set_gen_tstamp,  VALUE_TYPE_TSTAMP, v_any_ts, false },

    { "i.sus", Event_ins_set_sustain,         VALUE_TYPE_FLOAT, v_sustain, false },

    { "g.Bn",  Event_generator_set_bool_name, VALUE_TYPE_STRING, v_key, false },
    { "g.B",   Event_generator_set_bool,      VALUE_TYPE_BOOL, v_any_bool, false },
    { "g.In",  Event_generator_set_int_name,  VALUE_TYPE_STRING, v_key, false },
    { "g.I",   Event_generator_set_int,       VALUE_TYPE_INT, v_any_int, false },
    { "g.Fn",  Event_generator_set_float_name, VALUE_TYPE_STRING, v_key, false },
    { "g.F",   Event_generator_set_float,     VALUE_TYPE_FLOAT, v_any_float, false },
    { "g.Tn",  Event_generator_set_tstamp_name, VALUE_TYPE_STRING, v_key, false },
    { "g.T",   Event_generator_set_tstamp,    VALUE_TYPE_TSTAMP, v_any_ts, false },

    { "ebp+",  Event_effect_bypass_on,        VALUE_TYPE_NONE, NULL, false },
    { "ebp-",  Event_effect_bypass_off,       VALUE_TYPE_NONE, NULL, false },

    { "d.Bn",  Event_dsp_set_bool_name,       VALUE_TYPE_STRING, v_key, false },
    { "d.B",   Event_dsp_set_bool,            VALUE_TYPE_BOOL, v_any_bool, false },
    { "d.In",  Event_dsp_set_int_name,        VALUE_TYPE_STRING, v_key, false },
    { "d.I",   Event_dsp_set_int,             VALUE_TYPE_INT, v_any_int, false },
    { "d.Fn",  Event_dsp_set_float_name,      VALUE_TYPE_STRING, v_key, false },
    { "d.F",   Event_dsp_set_float,           VALUE_TYPE_FLOAT, v_any_float, false },
    { "d.Tn",  Event_dsp_set_tstamp_name,     VALUE_TYPE_STRING, v_key, false },
    { "d.T",   Event_dsp_set_tstamp,          VALUE_TYPE_TSTAMP, v_any_ts, false },

    { "qlocation", Event_query_location,      VALUE_TYPE_NONE, NULL, false },
    { "qvoices", Event_query_voice_count,     VALUE_TYPE_NONE, NULL, false },
    { "qf",    Event_query_actual_force,      VALUE_TYPE_INT, v_gen, false },

    { "Atrack", Event_auto_location_track,    VALUE_TYPE_INT, v_track, false },
    { "Asystem", Event_auto_location_system,  VALUE_TYPE_INT, v_system, false },
    { "Apattern", Event_auto_location_pattern, VALUE_TYPE_INT, v_pattern, false },
    { "Arow",  Event_auto_location_row,       VALUE_TYPE_TSTAMP, v_any_ts, false },
    { "Avoices", Event_auto_voice_count,      VALUE_TYPE_INT, v_any_int, false },
    { "Af",    Event_auto_actual_force,       VALUE_TYPE_FLOAT, v_any_float, false },

    { "", Event_NONE, VALUE_TYPE_NONE, NULL, false }
};


struct Event_names
{
    AAtree* names;
    bool error;
};


static int event_name_cmp(const char* e1, const char* e2);


Event_names* new_Event_names(void)
{
    Event_names* names = memory_alloc_item(Event_names);
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


Event_type Event_names_get(const Event_names* names, const char* name)
{
    assert(names != NULL);
    assert(name != NULL);

    Name_info* info = AAtree_get_exact(names->names, name);
    if (info == NULL)
    {
        return Event_NONE;
    }

    return info->type;
}


Value_type Event_names_get_param_type(
        const Event_names* names,
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
    memory_free(names);
    return;
}


