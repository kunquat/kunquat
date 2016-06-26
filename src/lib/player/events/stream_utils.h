

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


#ifndef KQT_STREAM_UTILS_H
#define KQT_STREAM_UTILS_H


#include <decl.h>

#include <stdlib.h>


/**
 * Find the Voice state of an active stream processor.
 *
 * \param channel       The Channel -- must not be \c NULL.
 * \param stream_name   The stream name -- must be a valid stream name.
 *
 * \return   The Voice state of the stream processor if found, otherwise
 *           \c NULL.
 */
Voice_state* get_target_stream_vstate(Channel* channel, const char* stream_name);


#endif // KQT_STREAM_UTILS_H


