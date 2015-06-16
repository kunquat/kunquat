

/*
 * Author: Tomi Jylhä-Ollila, Finland 2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_VERSION_H
#define KQT_VERSION_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Return the version of the Kunquat system.
 *
 * \return   The version string. This is also defined in version_def.h.
 */
const char* kqt_get_version(void);


#ifdef __cplusplus
}
#endif


#endif // KQT_VERSION_H


