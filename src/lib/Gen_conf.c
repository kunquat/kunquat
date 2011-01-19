

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2011
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <Device_params.h>
#include <Gen_conf.h>
#include <string_common.h>
#include <xassert.h>
#include <xmemory.h>


#define GENERATOR_DEFAULT_ENABLED (false)
#define GENERATOR_DEFAULT_VOLUME (0)
#define GENERATOR_DEFAULT_PITCH_LOCK_ENABLED (false)
#define GENERATOR_DEFAULT_PITCH_LOCK_CENTS (0)


static bool Gen_conf_parse_general(Gen_conf* conf,
                                   char* str,
                                   Read_state* state);


Gen_conf* new_Gen_conf(void)
{
    Gen_conf* conf = xalloc(Gen_conf);
    if (conf == NULL)
    {
        return NULL;
    }
    conf->params = NULL;
    conf->enabled = GENERATOR_DEFAULT_ENABLED;
    conf->volume_dB = GENERATOR_DEFAULT_VOLUME;
    conf->volume = exp2(conf->volume_dB / 6);
    conf->pitch_lock_enabled = GENERATOR_DEFAULT_PITCH_LOCK_ENABLED;
    conf->pitch_lock_cents = GENERATOR_DEFAULT_PITCH_LOCK_CENTS;
    conf->pitch_lock_freq = exp2(conf->pitch_lock_cents / 1200.0) * 440;

    conf->params = new_Device_params();
    if (conf->params == NULL)
    {
        del_Gen_conf(conf);
        return NULL;
    }
    return conf;
}


bool Gen_conf_parse(Gen_conf* conf,
                    const char* key,
                    void* data,
                    long length,
                    Device* device,
                    Read_state* state)
{
    assert(conf != NULL);
    assert(key != NULL);
    assert((data == NULL) == (length == 0));
    assert(length >= 0);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    if ((string_has_prefix(key, "i/") || string_has_prefix(key, "c/")) &&
            key_is_device_param(key))
    {
        if (!Device_params_parse_value(conf->params, key,
                                       data, length,
                                       state))
        {
            return false;
        }
        if (device != NULL)
        {
            return Device_update_key(device, key + 2);
        }
        return true;
    }
    else if (string_eq(key, "p_generator.json"))
    {
        return Gen_conf_parse_general(conf, data, state);
    }
    return true;
}


static bool Gen_conf_parse_general(Gen_conf* conf,
                                   char* str,
                                   Read_state* state)
{
    assert(conf != NULL);
    assert(state != NULL);
    if (state->error)
    {
        return false;
    }
    bool enabled = false;
    double volume = 0;
    bool pitch_lock_enabled = GENERATOR_DEFAULT_PITCH_LOCK_ENABLED;
    double pitch_lock_cents = GENERATOR_DEFAULT_PITCH_LOCK_CENTS;
    if (str != NULL)
    {
        str = read_const_char(str, '{', state);
        if (state->error)
        {
            return false;
        }
        str = read_const_char(str, '}', state);
        if (state->error)
        {
            Read_state_clear_error(state);
            char key[128] = { '\0' };
            bool expect_key = true;
            while (expect_key)
            {
                str = read_string(str, key, 128, state);
                str = read_const_char(str, ':', state);
                if (state->error)
                {
                    return false;
                }
                if (string_eq(key, "enabled"))
                {
                    str = read_bool(str, &enabled, state);
                }
                else if (string_eq(key, "volume"))
                {
                    str = read_double(str, &volume, state);
                }
                else if (string_eq(key, "pitch_lock"))
                {
                    str = read_bool(str, &pitch_lock_enabled, state);
                }
                else if (string_eq(key, "pitch_lock_cents"))
                {
                    str = read_double(str, &pitch_lock_cents, state);
                }
                else
                {
                    Read_state_set_error(state,
                             "Unsupported key in Generator info: %s", key);
                    return false;
                }
                if (state->error)
                {
                    return false;
                }
                check_next(str, state, expect_key);
            }
            str = read_const_char(str, '}', state);
            if (state->error)
            {
                return false;
            }
        }
    }
    conf->enabled = enabled;
    conf->volume_dB = volume;
    conf->volume = exp2(conf->volume_dB / 6);
    conf->pitch_lock_enabled = pitch_lock_enabled;
    conf->pitch_lock_cents = pitch_lock_cents;
    conf->pitch_lock_freq = exp2(conf->pitch_lock_cents / 1200.0) * 440;
    return true;
}


void del_Gen_conf(Gen_conf* conf)
{
    if (conf == NULL)
    {
        return;
    }
    del_Device_params(conf->params);
    xfree(conf);
    return;
}


