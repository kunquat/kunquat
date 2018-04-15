

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_LIMITS_FOREIGN_H
#define KQT_LIMITS_FOREIGN_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup Limits Kunquat limits
 * \{
 *
 * \brief
 * This module provides an interface for retrieving Kunquat-specific
 * boundary values described in kunquat/limits.h. It can be used to
 * eliminate duplicate definitions of these constants in foreign
 * language interfaces.
 */


/**
 * Return the names of all integer limit constants in libkunquat.
 *
 * \return   An array of the constant names. The names match the ones
 *           defined in kunquat/limits.h, excluding the KQT_ prefix.
 *           \c NULL marks the end of the array. The names are not
 *           listed in any particular order.
 */
const char** kqt_get_int_limit_names(void);


/**
 * Return the names of all string limit constants in libkunquat.
 *
 * \return   An array of the constant names. The names match the ones
 *           defined in kunquat/limits.h, excluding the KQT_ prefix.
 *           \c NULL marks the end of the array. The names are not
 *           listed in any particular order.
 */
const char** kqt_get_string_limit_names(void);


/**
 * Get integer limit constant.
 *
 * Note: This function is not optimised for performance; however, it is
 * safe to cache the returned information.
 *
 * \param limit_name   The name of the limit constant -- should be one
 *                     of the names returned by \a kqt_get_int_limit_names.
 *
 * \return   The limit constant, or \c 0 if \a limit_name is not valid.
 */
long long kqt_get_int_limit(const char* limit_name);


/**
 * Get string limit constant.
 *
 * Note: This function is not optimised for performance; however, it is
 * safe to cache the returned information.
 *
 * \param limit_name   The name of the limit constant -- should be one
 *                     of the names returned by \a kqt_get_string_limit_names.
 *
 * \return   The limit constant, or \c NULL if \a limit_name is not valid.
 */
const char* kqt_get_string_limit(const char* limit_name);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_LIMITS_FOREIGN_H


