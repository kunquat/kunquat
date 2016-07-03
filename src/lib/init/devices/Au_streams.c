

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Au_streams.h>

#include <containers/AAtree.h>
#include <debug/assert.h>
#include <kunquat/limits.h>
#include <memory.h>
#include <string/common.h>
#include <string/Streader.h>
#include <string/var_name.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>


typedef struct Entry
{
    char name[KQT_VAR_NAME_MAX];
    int proc_index;
} Entry;


struct Au_streams
{
    AAtree* tree;
};


Stream_target_dev_iter* Stream_target_dev_iter_init(
        Stream_target_dev_iter* iter, const Au_streams* streams)
{
    assert(iter != NULL);
    assert(streams != NULL);

    AAiter_init(&iter->iter, streams->tree);

    const Entry* entry = AAiter_get_at_least(&iter->iter, "");
    iter->next_name = (entry != NULL) ? entry->name : NULL;

    return iter;
}


const char* Stream_target_dev_iter_get_next(Stream_target_dev_iter* iter)
{
    assert(iter != NULL);

    if (iter->next_name == NULL)
        return NULL;

    const char* ret = iter->next_name;

    const Entry* entry = AAiter_get_next(&iter->iter);
    iter->next_name = (entry != NULL) ? entry->name : NULL;

    return ret;
}


static bool read_stream_entry(Streader* sr, int32_t index, void* userdata)
{
    assert(sr != NULL);
    ignore(index);

    Au_streams* streams = userdata;
    assert(streams != NULL);

    char stream_name[KQT_VAR_NAME_MAX + 1] = "";
    int64_t target_proc_index = -1;

    // Read stream name and target processor index
    if (!Streader_readf(
                sr, "[%s,%i]", KQT_VAR_NAME_MAX + 1, stream_name, &target_proc_index))
        return false;

    if (!is_valid_var_name(stream_name))
    {
        Streader_set_error(
                sr,
                "Illegal stream name %s"
                    " (Stream names may only contain"
                    " lower-case letters and underscores"
                    " (and digits as other than first characters))",
                stream_name);
        return false;
    }

    if ((target_proc_index < 0) || (target_proc_index >= KQT_PROCESSORS_MAX))
    {
        Streader_set_error(
                sr,
                "Invalid processor index as stream target of %s: %" PRId64,
                stream_name,
                target_proc_index);
        return false;
    }

    if (AAtree_contains(streams->tree, stream_name))
    {
        Streader_set_error(sr, "Duplicate stream entry: %s", stream_name);
        return false;
    }

    // Create and add new entry for the stream
    Entry* entry = memory_alloc_item(Entry);
    if (entry == NULL)
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for audio unit streams.");
        return false;
    }

    strcpy(entry->name, stream_name);
    entry->proc_index = (int)target_proc_index;

    if (!AAtree_ins(streams->tree, entry))
    {
        memory_free(entry);
        Streader_set_memory_error(
                sr, "Could not allocate memory for audio unit streams.");
        return false;
    }

    return true;
}


Au_streams* new_Au_streams(Streader* sr)
{
    Au_streams* streams = memory_alloc_item(Au_streams);
    if (streams == NULL)
        return NULL;

    streams->tree = NULL;

    streams->tree = new_AAtree(
            (AAtree_item_cmp*)strcmp, (AAtree_item_destroy*)memory_free);
    if (streams->tree == NULL)
    {
        del_Au_streams(streams);
        return NULL;
    }

    if (!Streader_read_list(sr, read_stream_entry, streams))
    {
        del_Au_streams(streams);
        return NULL;
    }

    return streams;
}


int Au_streams_get_target_proc_index(const Au_streams* streams, const char* stream_name)
{
    assert(streams != NULL);
    assert(stream_name != NULL);
    assert(is_valid_var_name(stream_name));

    const Entry* entry = AAtree_get_exact(streams->tree, stream_name);
    if (entry == NULL)
        return -1;

    return entry->proc_index;
}


void del_Au_streams(Au_streams* streams)
{
    if (streams == NULL)
        return;

    del_AAtree(streams->tree);
    memory_free(streams);

    return;
}


