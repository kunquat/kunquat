

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <memory.h>
#include <transient/Player.h>
#include <xassert.h>


struct Player
{
    const Module* module;
};


Player* new_Player(const Module* module)
{
    assert(module != NULL);

    Player* player = memory_alloc_item(Player);
    if (player == NULL)
        return NULL;

    player->module = NULL;

    player->module = module;

    return player;
}


void Player_play(Player* player, int32_t nframes)
{
    assert(player != NULL);
    assert(nframes >= 0);
    (void)player;
    (void)nframes;
    return;
}


int32_t Player_get_frames_available(Player* player)
{
    assert(player != NULL);
    (void)player;
    return 0;
}


void del_Player(Player* player)
{
    if (player == NULL)
        return;

    memory_free(player);
    return;
}


