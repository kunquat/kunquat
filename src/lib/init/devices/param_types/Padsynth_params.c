

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/param_types/Padsynth_params.h>

#include <containers/Vector.h>
#include <init/devices/param_types/Envelope.h>
#include <mathnum/common.h>
#include <memory.h>
#include <string/common.h>
#include <string/Streader.h>

#include <stdlib.h>


static bool read_harmonic(Streader* sr, int32_t index, void* userdata)
{
    rassert(sr != NULL);
    ignore(index);
    rassert(userdata != NULL);

    Vector* harmonics = userdata;

    Padsynth_harmonic* info =
        &(Padsynth_harmonic){ .freq_mul = NAN, .amplitude = NAN, .phase = NAN };
    if (!Streader_readf(sr, "[%f,%f", &info->freq_mul, &info->amplitude))
        return false;

    if (info->freq_mul <= 0)
    {
        Streader_set_error(
                sr, "PADsynth harmonic frequency multiplier must be positive");
        return false;
    }

    if (info->amplitude < 0)
    {
        Streader_set_error(sr, "PADsynth harmonic amplitude must be non-negative");
        return false;
    }

    if (Streader_match_char(sr, ']'))
    {
        info->phase = 0;
    }
    else
    {
        Streader_clear_error(sr);
        if (!Streader_readf(sr, ",%f]", &info->phase))
            return false;
    }

    if (!Vector_append(harmonics, info))
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for PADsynth harmonics");
        return false;
    }

    return true;
}


static bool read_param(Streader* sr, const char* key, void* userdata)
{
    rassert(sr != NULL);
    rassert(key != NULL);
    rassert(userdata != NULL);

    Padsynth_params* pp = userdata;

    if (string_eq(key, "sample_length"))
    {
        int64_t length = -1;
        if (!Streader_read_int(sr, &length))
            return false;

        if (!(PADSYNTH_MIN_SAMPLE_LENGTH <= length &&
                    length <= PADSYNTH_MAX_SAMPLE_LENGTH))
        {
            Streader_set_error(
                    sr,
                    "PADsynth sample length must be a power of 2"
                        " within range [%ld, %ld]",
                    PADSYNTH_MIN_SAMPLE_LENGTH,
                    PADSYNTH_MAX_SAMPLE_LENGTH);
            return false;
        }

        if (!is_p2(length))
        {
            Streader_set_error(sr, "PADsynth sample length must be a power of 2");
            return false;
        }

        pp->sample_length = (int32_t)length;
    }
    else if (string_eq(key, "audio_rate"))
    {
        int64_t rate = -1;
        if (!Streader_read_int(sr, &rate))
            return false;

        if (!(0 < rate && rate <= 1048576))
        {
            Streader_set_error(sr, "PADsynth sample rate must be positive"
                    " and not greater than 1048576");
            return false;
        }

        pp->audio_rate = (int32_t)rate;
    }
    else if (string_eq(key, "sample_count"))
    {
        int64_t sample_count = 0;
        if (!Streader_read_int(sr, &sample_count))
            return false;

        if (!(0 < sample_count && sample_count <= PADSYNTH_MAX_SAMPLE_COUNT))
        {
            Streader_set_error(
                    sr,
                    "PADsynth sample count must be within range [1, %d]",
                    PADSYNTH_MAX_SAMPLE_COUNT);
            return false;
        }

        pp->sample_count = (int)sample_count;
    }
    else if (string_eq(key, "pitch_range"))
    {
        double min_pitch = NAN;
        double max_pitch = NAN;
        if (!Streader_readf(sr, "[%f,%f]", &min_pitch, &max_pitch))
            return false;

        if (min_pitch > max_pitch)
        {
            Streader_set_error(
                    sr,
                    "PADsynth minimum sample map pitch must not be greater"
                        " than the maximum pitch");
            return false;
        }

        pp->min_pitch = min_pitch;
        pp->max_pitch = max_pitch;
    }
    else if (string_eq(key, "centre_pitch"))
    {
        double centre_pitch = NAN;
        if (!Streader_read_float(sr, &centre_pitch))
            return false;

        pp->centre_pitch = centre_pitch;
    }
    else if (string_eq(key, "bandwidth_base"))
    {
        double base = NAN;
        if (!Streader_read_float(sr, &base))
            return false;

        if (base <= 0)
        {
            Streader_set_error(sr, "PADsynth bandwidth base must be positive");
            return false;
        }

        pp->bandwidth_base = base;
    }
    else if (string_eq(key, "bandwidth_scale"))
    {
        double scale = NAN;
        if (!Streader_read_float(sr, &scale))
            return false;

        if (scale < 0)
        {
            Streader_set_error(sr, "PADsynth bandwidth scale must not be negative");
            return false;
        }

        pp->bandwidth_scale = scale;
    }
    else if (string_eq(key, "harmonics"))
    {
        if (!Streader_read_list(sr, read_harmonic, pp->harmonics))
            return false;

        if (Vector_size(pp->harmonics) == 0)
        {
            Streader_set_error(sr, "List of PADsynth harmonics is empty");
            return false;
        }
    }
    else if (string_eq(key, "use_phase_data"))
    {
        if (!Streader_read_bool(sr, &pp->use_phase_data))
            return false;
    }
    else if (string_eq(key, "phase_var_at_harmonic"))
    {
        double var = NAN;
        if (!Streader_read_float(sr, &var))
            return false;

        if ((var < 0) || (var > 1))
        {
            Streader_set_error(
                    sr,
                    "PADsynth phase variation at harmonic must be within range [0, 1]");
            return false;
        }

        pp->phase_var_at_harmonic = var;
    }
    else if (string_eq(key, "phase_var_off_harmonic"))
    {
        double var = NAN;
        if (!Streader_read_float(sr, &var))
            return false;

        if ((var < 0) || (var > 1))
        {
            Streader_set_error(
                    sr,
                    "PADsynth phase variation off harmonic must be within range [0, 1]");
            return false;
        }

        pp->phase_var_off_harmonic = var;
    }
    else if (string_eq(key, "phase_spread_bandwidth_base"))
    {
        double base = NAN;
        if (!Streader_read_float(sr, &base))
            return false;

        if (base <= 0)
        {
            Streader_set_error(
                    sr, "PADsynth phase spread bandwidth base must be positive");
            return false;
        }

        pp->phase_spread_bandwidth_base = base;
    }
    else if (string_eq(key, "phase_spread_bandwidth_scale"))
    {
        double scale = NAN;
        if (!Streader_read_float(sr, &scale))
            return false;

        if (scale < 0)
        {
            Streader_set_error(
                    sr, "PADsynth phase spread bandwidth scale must not be negative");
            return false;
        }

        pp->phase_spread_bandwidth_scale = scale;
    }
    else if (string_eq(key, "res_env_enabled"))
    {
        if (!Streader_read_bool(sr, &pp->is_res_env_enabled))
            return false;
    }
    else if (string_eq(key, "res_env"))
    {
        if (pp->res_env != NULL)
        {
            Streader_set_error(
                    sr, "Multiple resonance envelope entries in PADsynth parameters");
            return false;
        }

        const double freq_limit = PADSYNTH_DEFAULT_AUDIO_RATE / 2;

        Envelope* env = new_Envelope(64, 0, freq_limit, 0, 0, 64, 0);
        if (env == NULL)
        {
            Streader_set_memory_error(
                    sr, "Could not allocate memory for PADsynth resonance envelope");
            return false;
        }

        if (!Envelope_read(env, sr))
        {
            rassert(Streader_is_error_set(sr));
            del_Envelope(env);
            return false;
        }

        {
            if (Envelope_get_node(env, 0)[0] != 0)
            {
                Streader_set_error(
                        sr, "PADsynth resonance envelope does not start at frequency 0");
                del_Envelope(env);
                return false;
            }

            const int node_count = Envelope_node_count(env);
            if (Envelope_get_node(env, node_count - 1)[0] != freq_limit)
            {
                Streader_set_error(
                        sr,
                        "PADsynth resonance envelope does not end at frequency %.0f",
                        freq_limit);
                del_Envelope(env);
                return false;
            }
        }

        pp->res_env = env;
    }
    else
    {
        Streader_set_error(
                sr, "Unrecognised key in PADsynth parameters: %s", key);
        return false;
    }

    return true;
}


Padsynth_params* new_Padsynth_params(Streader* sr, int version)
{
    rassert(sr != NULL);
    rassert(version >= 0);
    rassert(version <= 1);

    if (Streader_is_error_set(sr))
        return NULL;

    Padsynth_params* pp = memory_alloc_item(Padsynth_params);
    if (pp == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for PADsynth parameters");
        return NULL;
    }

    pp->is_res_env_enabled = false;
    pp->res_env = NULL;

    pp->harmonics = new_Vector(sizeof(Padsynth_harmonic));
    if (pp->harmonics == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for PADsynth parameters");
        del_Padsynth_params(pp);
        return NULL;
    }

    pp->sample_length = PADSYNTH_DEFAULT_SAMPLE_LENGTH;
    pp->audio_rate = PADSYNTH_DEFAULT_AUDIO_RATE;
    pp->sample_count = 1;
    pp->min_pitch = 0;
    pp->max_pitch = 0;
    pp->centre_pitch = 0;

    pp->bandwidth_base = PADSYNTH_DEFAULT_BANDWIDTH_BASE;
    pp->bandwidth_scale = PADSYNTH_DEFAULT_BANDWIDTH_SCALE;

    pp->use_phase_data = false;
    pp->phase_var_at_harmonic = PADSYNTH_DEFAULT_PHASE_VAR_AT_HARMONIC;
    pp->phase_var_off_harmonic = PADSYNTH_DEFAULT_PHASE_VAR_OFF_HARMONIC;
    pp->phase_spread_bandwidth_base = PADSYNTH_DEFAULT_PHASE_SPREAD_BW_BASE;
    pp->phase_spread_bandwidth_scale = PADSYNTH_DEFAULT_PHASE_SPREAD_BW_SCALE;

    if (!Streader_has_data(sr))
    {
        Padsynth_harmonic* info =
            &(Padsynth_harmonic){ .freq_mul = 1, .amplitude = 1, .phase = 0 };
        if (!Vector_append(pp->harmonics, info))
        {
            Streader_set_error(sr, "Could not allocate memory for PADsynth harmonics");
            del_Padsynth_params(pp);
            return NULL;
        }

        return pp;
    }

    if (!Streader_read_dict(sr, read_param, pp))
    {
        del_Padsynth_params(pp);
        return NULL;
    }

    if (Vector_size(pp->harmonics) == 0)
    {
        Streader_set_error(sr, "No harmonics found in PADsynth parameters");
        del_Padsynth_params(pp);
        return NULL;
    }

    return pp;
}


void del_Padsynth_params(Padsynth_params* pp)
{
    if (pp == NULL)
        return;

    del_Envelope(pp->res_env);
    del_Vector(pp->harmonics);
    memory_free(pp);

    return;
}


