

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_AU_STREAMS_H
#define KQT_AU_STREAMS_H


#include <containers/AAtree.h>
#include <decl.h>
#include <string/Streader.h>

#include <stdlib.h>


typedef struct Stream_target_dev_iter
{
    AAiter iter;
    const char* next_name;
} Stream_target_dev_iter;


#define STREAM_TARGET_DEV_ITER_AUTO (&(Stream_target_dev_iter){ *AAITER_AUTO, NULL })


/**
 * A mapping from stream names to target devices.
 */
//typedef struct Au_streams Au_streams;


/**
 * Initialise a Stream target device iterator.
 *
 * \param iter      The Stream target device iterator -- must not be \c NULL.
 * \param streams   The Audio unit stream map -- must not be \c NULL.
 *
 * \return   The parameter \a iter.
 */
Stream_target_dev_iter* Stream_target_dev_iter_init(
        Stream_target_dev_iter* iter, const Au_streams* streams);


/**
 * Get the next Stream target name from the iterator.
 *
 * \param iter   The Stream target device iterator -- must not be \c NULL.
 *
 * \return   The next info structure, or \c NULL if reached the end of iteration.
 */
const char* Stream_target_dev_iter_get_next(Stream_target_dev_iter* iter);


/**
 * Create new Audio unit stream map.
 *
 * \param sr   The Streader of the JSON input -- must not be \c NULL.
 *
 * \return   The new Audio unit stream map if successful, otherwise \c NULL.
 */
Au_streams* new_Au_streams(Streader* sr);


/**
 * Get target processor index from the Audio unit stream map.
 *
 * \param streams       The Audio unit stream map -- must not be \c NULL.
 * \param stream_name   The name of the stream -- must be a valid stream name.
 *
 * \return   The target processor index, or \c -1 if there is no stream
 *           called \a stream_name in \a streams.
 */
int Au_streams_get_target_proc_index(const Au_streams* streams, const char* stream_name);


/**
 * Destroy an existing Audio unit stream map.
 *
 * \param streams   The Audio unit stream map, or \c NULL.
 */
void del_Au_streams(Au_streams* streams);


#endif // KQT_AU_STREAMS_H


