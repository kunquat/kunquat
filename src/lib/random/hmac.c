

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <debug/assert.h>
#include <random/hmac.h>
#include <random/md5.h>


void hmac_md5(uint64_t key, const char* msg, uint64_t* lower, uint64_t* upper)
{
    assert(msg != NULL);
    assert(lower != NULL);
    assert(upper != NULL);

    unsigned char key_str[64] = { 0 };
    for (int i = 0; i < 8; ++i)
        key_str[i] = (key >> 8 * i) & 0xff;

    for (int i = 0; i < 64; ++i)
        key_str[i] ^= 0x36;

    uint64_t hlower = 0;
    uint64_t hupper = 0;
    md5((char*)key_str, 64, &hlower, &hupper, false);
    md5_with_state(
            msg, strlen(msg),
            &hlower, &hupper,
            hlower, hupper,
            true,
            64);

    unsigned char hkey[16] = { 0 };
    for (int i = 0; i < 8; ++i)
    {
        hkey[i] = hlower & 0xff;
        hkey[i + 8] = hupper & 0xff;
        hlower >>= 8;
        hupper >>= 8;
    }

    memset(key_str, 0, 64);
    for (int i = 0; i < 8; ++i)
        key_str[i] = (key >> 8 * i) & 0xff;
    for (int i = 0; i < 64; ++i)
        key_str[i] ^= 0x5c;

    md5((char*)key_str, 64, &hlower, &hupper, false);
    md5_with_state(
            (char*)hkey, 16,
            lower, upper,
            hlower, hupper,
            true, 64);

    return;
}


