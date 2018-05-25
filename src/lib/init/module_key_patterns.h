

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/comp_defaults.h>
#include <init/sheet/song_defaults.h>


#ifndef MODULE_KEYP
#error "MODULE_KEYP(name, keyp, version, def_val) not defined"
#endif


#ifdef MAKE_STRING
#error "MAKE_STRING already defined"
#endif
#ifdef MAKE_STRING2
#error "MAKE_STRING2 already defined"
#endif
#define MAKE_STRING2(x) #x
#define MAKE_STRING(x) MAKE_STRING2(x)


MODULE_KEYP(dc_blocker_enabled,     "p_dc_blocker_enabled.json",        0,  "true")
MODULE_KEYP(mixing_volume, "p_mixing_volume.json", 0, MAKE_STRING(COMP_DEFAULT_MIX_VOL))
MODULE_KEYP(force_shift,            "p_force_shift.json",               0,  "0")
MODULE_KEYP(out_port_manifest,      "out_XX/p_manifest.json",           0,  "")
MODULE_KEYP(connections,            "p_connections.json",               0,  "[]")
MODULE_KEYP(control_map,            "p_control_map.json",               0,  "[]")
MODULE_KEYP(control_manifest,       "control_XX/p_manifest.json",       0,  "")
MODULE_KEYP(random_seed,            "p_random_seed.json",               0,  "0")
MODULE_KEYP(environment,            "p_environment.json",               0,  "[]")
MODULE_KEYP(bind,                   "p_bind.json",                      0,  "[]")
MODULE_KEYP(ch_defaults, "p_channel_defaults.json", 0,
        "[{ \"control\": 0"
        " , \"init_expr\": \"\" }]")

MODULE_KEYP(album_manifest,         "album/p_manifest.json",            0,  "")
MODULE_KEYP(album_tracks,           "album/p_tracks.json",              0,  "[]")

#ifndef MODULE_AU_KEYP
#define MODULE_AU_KEYP(name, keyp, version, def_val)                 \
    MODULE_KEYP(name,           keyp,           version,    def_val) \
    MODULE_KEYP(au_ ## name,    "au_XX/" keyp,  version,    def_val)
#endif

MODULE_AU_KEYP(au_manifest,             "au_XX/p_manifest.json",                0,  "")
MODULE_AU_KEYP(au_in_port_manifest,     "au_XX/in_XX/p_manifest.json",          0,  "")
MODULE_AU_KEYP(au_out_port_manifest,    "au_XX/out_XX/p_manifest.json",         0,  "")
MODULE_AU_KEYP(au_connections,          "au_XX/p_connections.json",             0,  "[]")

MODULE_AU_KEYP(au_streams,              "au_XX/p_streams.json",                 0,  "[]")
MODULE_AU_KEYP(au_events,               "au_XX/p_events.json",                  0,  "[]")
MODULE_AU_KEYP(au_hit_manifest,         "au_XX/hit_XX/p_manifest.json",         0,  "")
MODULE_AU_KEYP(au_hit_proc_filter,      "au_XX/hit_XX/p_hit_proc_filter.json",  0,  "[]")
MODULE_AU_KEYP(au_expressions,          "au_XX/p_expressions.json",             0,  "{}")

MODULE_AU_KEYP(proc_manifest,           "au_XX/proc_XX/p_manifest.json",        0,  "")
MODULE_AU_KEYP(proc_signal_type,        "au_XX/proc_XX/p_signal_type.json",     0,  "\"voice\"")
MODULE_AU_KEYP(proc_in_port_manifest,   "au_XX/proc_XX/in_XX/p_manifest.json",  0,  "")
MODULE_AU_KEYP(proc_out_port_manifest,  "au_XX/proc_XX/out_XX/p_manifest.json", 0,  "")
MODULE_AU_KEYP(proc_impl_key,           "au_XX/proc_XX/i/",                     0,  "")
MODULE_AU_KEYP(proc_conf_key,           "au_XX/proc_XX/c/",                     0,  "")

#undef MODULE_AU_KEYP

MODULE_KEYP(pattern_manifest,   "pat_XXX/p_manifest.json",          0,  "")
MODULE_KEYP(pattern_length,     "pat_XXX/p_length.json",            0,  "[16, 0]")
MODULE_KEYP(column,             "pat_XXX/col_XX/p_triggers.json",   0,  "[]")

MODULE_KEYP(pat_instance_manifest, "pat_XXX/instance_XXX/p_manifest.json", 0, "")

MODULE_KEYP(tuning_table, "tuning_XX/p_tuning_table.json", 0,
        "{ \"ref_note\": 0"
        ", \"ref_pitch\": 0"
        ", \"pitch_offset\": 0"
        ", \"octave_width\": [2, 1]"
        ", \"centre_octave\": 4"
        ", \"notes\": [0]"
        "}")

MODULE_KEYP(song_manifest,   "song_XX/p_manifest.json",   0, "")
MODULE_KEYP(song_tempo,      "song_XX/p_tempo.json",      0, MAKE_STRING(SONG_DEFAULT_TEMPO))
MODULE_KEYP(song_order_list, "song_XX/p_order_list.json", 0, "[]")


#undef MAKE_STRING
#undef MAKE_STRING2


#undef MODULE_KEYP


