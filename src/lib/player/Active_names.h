

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_ACTIVE_NAMES_H
#define K_ACTIVE_NAMES_H


#include <stdbool.h>


typedef enum Active_cat
{
    ACTIVE_CAT_ENV = 0,
    ACTIVE_CAT_LAST
} Active_cat;


typedef enum Active_type
{
    ACTIVE_TYPE_BOOL = 0,
    ACTIVE_TYPE_INT,
    ACTIVE_TYPE_FLOAT,
    ACTIVE_TYPE_TSTAMP,
    ACTIVE_TYPE_LAST
} Active_type;


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
 * \param cat     The category -- must be valid.
 * \param type    The variable type -- must be valid.
 * \param name    The new name -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if \a name contains too
 *           many characters to fit the type.
 */
bool Active_names_set(
        Active_names* names,
        Active_cat cat,
        Active_type type,
        const char* name);


/**
 * Get an active name for a type.
 *
 * \param names   The Active names -- must not be \c NULL.
 * \param cat     The category -- must be valid.
 * \param type    The variable type -- must be valid.
 *
 * \return   The active name. This is never \c NULL.
 */
const char* Active_names_get(
        const Active_names* names, Active_cat cat, Active_type type);


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


#endif // K_ACTIVE_NAMES_H


