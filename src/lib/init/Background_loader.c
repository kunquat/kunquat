

/*
 * Author: Tomi Jylhä-Ollila, Finland 2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <debug/assert.h>
#include <Error.h>
#include <init/Background_loader.h>
#include <memory.h>

#include <stdbool.h>
#include <stdlib.h>


struct Background_loader
{
    int thread_count;
};


Background_loader* new_Background_loader(void)
{
    Background_loader* loader = memory_alloc_item(Background_loader);
    if (loader == NULL)
        return NULL;

    loader->thread_count = 0;

    return loader;
}


bool Background_loader_set_thread_count(
        Background_loader* loader, int count, Error* error)
{
    rassert(loader != NULL);
    rassert(count >= 0);
    rassert(error != NULL);

    // TODO
#if 0
    loader->thread_count = count;
#else
    loader->thread_count = 0;
#endif

    return true;
}


int Background_loader_get_thread_count(const Background_loader* loader)
{
    rassert(loader != NULL);
    return loader->thread_count;
}


bool Background_loader_add_task(Background_loader* loader, Background_loader_task* task)
{
    rassert(loader != NULL);
    rassert(task != NULL);
    rassert(task->callback != NULL);

    task->is_finished = false;
    // TODO: Get error destination
    //Error_clear(task->error);

    if (loader->thread_count > 0)
    {
        // TODO
    }

    return false;
}


void Background_loader_wait_idle(Background_loader* loader)
{
    rassert(loader != NULL);

    // TODO

    return;
}


Error* Background_loader_get_first_error(const Background_loader* loader)
{
    rassert(loader != NULL);

    // TODO

    return NULL;
}


void Background_loader_reset(Background_loader* loader)
{
    rassert(loader != NULL);

    // TODO

    return;
}


void del_Background_loader(Background_loader* loader)
{
    if (loader == NULL)
        return;

    memory_free(loader);

    return;
}


