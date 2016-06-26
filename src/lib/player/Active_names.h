

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_ACTIVE_NAMES_H
#define KQT_ACTIVE_NAMES_H


#include <stdbool.h>
#include <stdlib.h>


typedef enum Active_cat
{
    ACTIVE_CAT_ENV = 0,
    ACTIVE_CAT_CONTROL_VAR,
    ACTIVE_CAT_STREAM,
    ACTIVE_CAT_INIT_EXPRESSION,
    ACTIVE_CAT_EXPRESSION,
    ACTIVE_CAT_COUNT
} Active_cat;


/**
 * Names of variables that are subject to modification.
 */
typedef struct Active_names Active_names;


/**
 * Create new Active names.
 *
 * \return   The new Active names if successful, or \c NULL if memory
 *           allocation failed.
 */
Active_names* new_Active_names(void);


/**
 * Set an active name for a type.
 *
 * \param names   The Active names -- must not be \c NULL.
 * \param cat     The category -- must be < \c ACTIVE_CAT_COUNT.
 * \param name    The new name -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if \a name contains too
 *           many characters to fit the type.
 */
bool Active_names_set(Active_names* names, Active_cat cat, const char* name);


/**
 * Get an active name for a type.
 *
 * \param names   The Active names -- must not be \c NULL.
 * \param cat     The category -- must be < \c ACTIVE_CAT_COUNT.
 *
 * \return   The active name. This is never \c NULL.
 */
const char* Active_names_get(const Active_names* names, Active_cat cat);


/**
 * Reset the Active names.
 *
 * \param names   The Active names -- must not be \c NULL.
 */
void Active_names_reset(Active_names* names);


/**
 * Destroy existing Active names.
 *
 * \param names   The Active names, or \c NULL.
 */
void del_Active_names(Active_names* names);


#endif // KQT_ACTIVE_NAMES_H


