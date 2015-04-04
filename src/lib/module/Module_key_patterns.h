

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <Decl.h>
#include <module/sheet/Song.h>
#include <module/Module.h>


#ifndef MODULE_KEYP
#error "MODULE_KEYP(name, keyp, def_val) not defined"
#endif


#ifdef MAKE_STRING
#error "MAKE_STRING already defined"
#endif
#ifdef MAKE_STRING2
#error "MAKE_STRING2 already defined"
#endif
#define MAKE_STRING2(x) #x
#define MAKE_STRING(x) MAKE_STRING2(x)


MODULE_KEYP(composition, "p_composition.json",
        "{ \"mix_vol\": " MAKE_STRING(MODULE_DEFAULT_MIX_VOL) " }")
MODULE_KEYP(out_port_manifest,      "out_XX/p_manifest.json",               "")
MODULE_KEYP(connections,            "p_connections.json",                   "[]")
MODULE_KEYP(control_map,            "p_control_map.json",                   "[]")
MODULE_KEYP(control_manifest,       "control_XX/p_manifest.json",           "")
MODULE_KEYP(random_seed,            "p_random_seed.json",                   "0")
MODULE_KEYP(environment,            "p_environment.json",                   "[]")
MODULE_KEYP(bind,                   "p_bind.json",                          "[]")

MODULE_KEYP(album_manifest,         "album/p_manifest.json",                "")
MODULE_KEYP(album_tracks,           "album/p_tracks.json",                  "[]")

#ifndef MODULE_AU_KEYP
#define MODULE_AU_KEYP(name, keyp, def_val)                  \
    MODULE_KEYP(name,           keyp,               def_val) \
    MODULE_KEYP(au_ ## name,    "au_XX/" keyp,      def_val)
#endif

MODULE_AU_KEYP(au_manifest,             "au_XX/p_manifest.json",                "")
MODULE_AU_KEYP(au,                      "au_XX/p_audio_unit.json",
        "{ \"global_force\": 0"
        ", \"force\": 0"
        ", \"force_variation\": 0"
        "}")
MODULE_AU_KEYP(au_in_port_manifest,     "au_XX/in_XX/p_manifest.json",          "")
MODULE_AU_KEYP(au_out_port_manifest,    "au_XX/out_XX/p_manifest.json",         "")
MODULE_AU_KEYP(au_connections,          "au_XX/p_connections.json",             "[]")
MODULE_AU_KEYP(au_env_force,            "au_XX/p_envelope_force.json",
        "{ \"enabled\": false"
        ", \"scale_amount\": 0"
        ", \"scale_center\": 0"
        ", \"loop\": false"
        ", \"envelope\": { \"nodes\": [ [0, 1], [1, 1] ], \"marks\": [0, 1] }"
        "}")
MODULE_AU_KEYP(au_env_force_release,    "au_XX/p_envelope_force_release.json",
        "{ \"enabled\": false"
        ", \"scale_amount\": 0"
        ", \"scale_center\": 0"
        ", \"envelope\": { \"nodes\": [ [0, 1], [1, 0] ] }"
        "}")
MODULE_AU_KEYP(au_env_force_filter,     "au_XX/p_envelope_force_filter.json",   "")
MODULE_AU_KEYP(au_env_pitch_pan,        "au_XX/p_envelope_pitch_pan.json",      "")

MODULE_AU_KEYP(proc_manifest,           "au_XX/proc_XX/p_manifest.json",        "")
MODULE_AU_KEYP(proc_signal_type,        "au_XX/proc_XX/p_signal_type.json",     "\"voice\"")
MODULE_AU_KEYP(proc_in_port_manifest,   "au_XX/proc_XX/in_XX/p_manifest.json",  "")
MODULE_AU_KEYP(proc_out_port_manifest,  "au_XX/proc_XX/out_XX/p_manifest.json", "")
MODULE_AU_KEYP(proc_vf_pitch,           "au_XX/proc_XX/out_XX/p_vf_pitch.json", "true")
MODULE_AU_KEYP(proc_vf_force,           "au_XX/proc_XX/out_XX/p_vf_force.json", "true")
MODULE_AU_KEYP(proc_vf_cut,             "au_XX/proc_XX/out_XX/p_vf_cut.json",   "true")
MODULE_AU_KEYP(proc_vf_filter,          "au_XX/proc_XX/out_XX/p_vf_filter.json", "true")
MODULE_AU_KEYP(proc_vf_panning,         "au_XX/proc_XX/out_XX/p_vf_panning.json", "true")
MODULE_AU_KEYP(proc_impl_key,           "au_XX/proc_XX/i/",                     "")
MODULE_AU_KEYP(proc_conf_key,           "au_XX/proc_XX/c/",                     "")

#undef MODULE_AU_KEYP

MODULE_KEYP(pattern_manifest, "pat_XXX/p_manifest.json", "")
MODULE_KEYP(pattern,          "pat_XXX/p_pattern.json", "{ \"length\": [16, 0] }")
MODULE_KEYP(column,           "pat_XXX/col_XX/p_triggers.json", "[]")

MODULE_KEYP(pat_instance_manifest, "pat_XXX/instance_XXX/p_manifest.json", "")

MODULE_KEYP(scale, "scale_X/p_scale.json",
        "{ \"ref_note\": 0"
        ", \"ref_pitch\": 0"
        ", \"pitch_offset\": 0"
        ", \"octave_ratio\": [2, 1]"
        ", \"notes\": []"
        "}")

MODULE_KEYP(song_manifest,      "song_XX/p_manifest.json",          "")
MODULE_KEYP(song,               "song_XX/p_song.json",
        "{ \"tempo\": " MAKE_STRING(SONG_DEFAULT_TEMPO)
        ", \"global_vol\": " MAKE_STRING(SONG_DEFAULT_GLOBAL_VOL)
        ", \"scale\": " MAKE_STRING(SONG_DEFAULT_SCALE)
        "}")
MODULE_KEYP(song_order_list,    "song_XX/p_order_list.json",        "[]")
MODULE_KEYP(song_ch_defaults,   "song_XX/p_channel_defaults.json",
        "[{ \"control\": 0 }]")


#undef MAKE_STRING
#undef MAKE_STRING2


#undef MODULE_KEYP


