

/*
 * Author: Tomi Jylhä-Ollila, Finland 2014-2018
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_EVENTS_H
#define KQT_EVENTS_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * \defgroup Events Kunquat events
 * \{
 *
 * \brief
 * This module describes an interface for retrieving libkunquat event
 * information.
 */


/**
 * Return the names of all events supported by libkunquat.
 *
 * \return   An array of the event names. \c NULL marks the end of the
 *           array. The names are not listed in any particular order.
 */
const char** kqt_get_event_names(void);


/**
 * Get event argument type description.
 *
 * The argument type is represented by one of the following:
 *
 * \li "bool"
 * \li "int"
 * \li "float"
 * \li "tstamp" (timestamp; JSON format: [x, y]; expression format:
 *     ts(x, y))
 * \li "string"
 * \li "pat" (pattern instance reference; JSON format: [x, y];
 *     expression format: pat(x, y))
 * \li "pitch" (a floating-point value as an offset from 440 Hz in cents;
 *     however, it is often recommended to use note names for display)
 * \li NULL (no argument)
 *
 * Note: This function is not optimised for performance; however, it is
 * safe to cache the returned information.
 *
 * \param event_name   The name of the event -- should be one of the
 *                     names returned by \a kqt_get_event_names.
 *
 * \return   The event argument type as described above, or \c NULL if
 *           \a event_name is not supported.
 */
const char* kqt_get_event_arg_type(const char* event_name);


/**
 * Get event name specifier that matches given event name.
 *
 * \param event_name   The name of the event -- should be one of the
 *                     names returned by \a kqt_get_event_names.
 *
 * \return   The event name specifier if one exists, otherwise \c NULL.
 */
const char* kqt_get_event_name_specifier(const char* event_name);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_EVENTS_H


