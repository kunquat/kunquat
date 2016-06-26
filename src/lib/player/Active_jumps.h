

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_ACTIVE_JUMPS_H
#define KQT_ACTIVE_JUMPS_H


#include <containers/AAtree.h>
#include <kunquat/limits.h>
#include <mathnum/Tstamp.h>
#include <player/Jump_cache.h>
#include <player/Jump_context.h>
#include <Pat_inst_ref.h>

#include <stdlib.h>


/**
 * A collection of active Jump contexts.
 */
typedef struct Active_jumps Active_jumps;


/**
 * Create new Active jumps.
 *
 * The collection is initially empty -- the Jump cache provides the Jump
 * contexts.
 *
 * \return   The new Active jumps if successful, or \c NULL if memory
 *           allocation failed.
 */
Active_jumps* new_Active_jumps(void);


/**
 * Add a Jump context handle to the Active jumps.
 *
 * \param jumps    The Active jumps -- must not be \c NULL.
 * \param handle   The Jump context handle -- must not be \c NULL and must
 *                 not match another active Jump context.
 */
void Active_jumps_add_context(Active_jumps* jumps, AAnode* handle);


/**
 * Find the next Jump context in the Active jumps.
 *
 * \param jumps    The Active jumps -- must not be \c NULL.
 * \param piref    The current pattern instance -- must not be \c NULL.
 * \param row      The minimum row timestamp -- must not be \c NULL.
 * \param ch_num   Minimum channel number -- must be >= \c 0 and
 *                 < \c KQT_CHANNELS_MAX. Note: Counters of externally fired
 *                 jump events use KQT_CHANNELS_MAX as their channel number.
 * \param order    Minimum order index -- must be >= \c 0.
 *
 * \return   The next Jump context handle if one is located inside the given
 *           pattern instance, otherwise \c NULL.
 */
Jump_context* Active_jumps_get_next_context(
        const Active_jumps* jumps,
        const Pat_inst_ref* piref,
        const Tstamp* row,
        int ch_num,
        int order);


/**
 * Remove a Jump context handle from the Active jumps.
 *
 * \param jumps   The Active jumps -- must not be \c NULL.
 * \param jc      The Jump context -- must not be \c NULL and must match
 *                an active Jump context.
 *
 * \return   The Jump context handle.
 */
AAnode* Active_jumps_remove_context(Active_jumps* jumps, const Jump_context* jc);


/**
 * Move all Jump context handles from the Active jumps to the Jump cache.
 *
 * \param jumps    The Active jumps -- must not be \c NULL.
 * \param jcache   The Jump cache -- must not be \c NULL.
 */
void Active_jumps_reset(Active_jumps* jumps, Jump_cache* jcache);


/**
 * Destroy existing Active jumps.
 *
 * \param jumps   The Active jumps, or \c NULL.
 */
void del_Active_jumps(Active_jumps* jumps);


#endif // KQT_ACTIVE_JUMPS_H


