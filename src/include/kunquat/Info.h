

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_INFO_H
#define KQT_INFO_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/Handle.h>
#include <kunquat/limits.h>


/**
 * Gets the length of a Subsong in the Kunquat Handle.
 *
 * \param handle    The Handle -- should not be \c NULL.
 * \param subsong   The Subsong number -- should be >= \c -1 and
 *                  < \c KQT_SUBSONGS_MAX. Using \c -1 will return the total
 *                  number of sections in all Subsongs.
 *
 * \return   The length of the Subsong, or \c -1 if arguments were invalid.
 */
int kqt_Handle_get_subsong_length(kqt_Handle* handle, int subsong);


#ifdef __cplusplus
}
#endif


#endif // KQT_INFO_H


