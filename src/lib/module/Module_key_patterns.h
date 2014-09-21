

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


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
MODULE_KEYP(connections,            "p_connections.json",                   "[]")
MODULE_KEYP(control_map,            "p_control_map.json",                   "[]")
MODULE_KEYP(control_manifest,       "control_XX/p_manifest.json",           "")
MODULE_KEYP(random_seed,            "p_random_seed.json",                   "0")
MODULE_KEYP(environment,            "p_environment.json",                   "[]")
MODULE_KEYP(bind,                   "p_bind.json",                          "[]")

MODULE_KEYP(album_manifest,         "album/p_manifest.json",                "")
MODULE_KEYP(album_tracks,           "album/p_tracks.json",                  "[]")

MODULE_KEYP(ins_manifest,           "ins_XX/p_manifest.json",               "")
MODULE_KEYP(ins,                    "ins_XX/p_instrument.json",             "")
MODULE_KEYP(ins_connections,        "ins_XX/p_connections.json",            "[]")
MODULE_KEYP(ins_env_force,          "ins_XX/p_envelope_force.json",         "")
MODULE_KEYP(ins_env_force_release,  "ins_XX/p_envelope_force_release.json", "")
MODULE_KEYP(ins_env_force_filter,   "ins_XX/p_envelope_force_filter.json",  "")
MODULE_KEYP(ins_env_pitch_pan,      "ins_XX/p_envelope_pitch_pan.json",     "")

MODULE_KEYP(gen_manifest,           "ins_XX/gen_XX/p_manifest.json",        "")
MODULE_KEYP(gen_type,               "ins_XX/gen_XX/p_gen_type.json",        "")
MODULE_KEYP(gen_impl_key,           "ins_XX/gen_XX/i/",                     "")
MODULE_KEYP(gen_conf_key,           "ins_XX/gen_XX/c/",                     "")

MODULE_KEYP(ins_effect_manifest,    "ins_XX/eff_XX/p_manifest.json",        "")
MODULE_KEYP(ins_effect_connections, "ins_XX/eff_XX/p_connections.json",     "")
MODULE_KEYP(ins_dsp_manifest,       "ins_XX/eff_XX/dsp_XX/p_manifest.json", "")
MODULE_KEYP(ins_dsp_type,           "ins_XX/eff_XX/dsp_XX/p_dsp_type.json", "")
MODULE_KEYP(ins_dsp_impl_key,       "ins_XX/eff_XX/dsp_XX/i/",              "")
MODULE_KEYP(ins_dsp_conf_key,       "ins_XX/eff_XX/dsp_XX/c/",              "")

MODULE_KEYP(effect_manifest,        "eff_XX/p_manifest.json",               "")
MODULE_KEYP(effect_connections,     "eff_XX/p_connections.json",            "[]")
MODULE_KEYP(dsp_manifest,           "eff_XX/dsp_XX/p_manifest.json",        "")
MODULE_KEYP(dsp_type,               "eff_XX/dsp_XX/p_dsp_type.json",        "")
MODULE_KEYP(dsp_impl_key,           "eff_XX/dsp_XX/i/",                     "")
MODULE_KEYP(dsp_conf_key,           "eff_XX/dsp_XX/c/",                     "")

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

MODULE_KEYP(song_manifest,   "song_XX/p_manifest.json",   "")
MODULE_KEYP(song,            "song_XX/p_song.json",
        "{ \"tempo\": " MAKE_STRING(SONG_DEFAULT_TEMPO)
        ", \"global_vol\": " MAKE_STRING(SONG_DEFAULT_GLOBAL_VOL)
        ", \"scale\": " MAKE_STRING(SONG_DEFAULT_SCALE)
        "}")
MODULE_KEYP(song_order_list, "song_XX/p_order_list.json", "[]")


#undef MAKE_STRING
#undef MAKE_STRING2


#undef MODULE_KEYP


