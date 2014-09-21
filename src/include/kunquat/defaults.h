

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DEFAULTS_H
#define KQT_DEFAULTS_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup Defaults Default values
 * \{
 *
 * \brief
 * This module describes the interface for retrieving various default values.
 */


/**
 * Return the default composition data value associated with the given key.
 *
 * \param key   The key -- should not be \c NULL. The function does not check
 *              ranges of any numbers contained by the key; however, the
 *              number of digits in those numbers is still significant.
 *
 * \return   The default value as a JSON string, or an empty string if \a key
 *           is not recognised or does not have an associated default value.
 */
const char* kqt_get_default_value(const char* key);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_DEFAULTS_H


