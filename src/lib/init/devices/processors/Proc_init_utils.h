

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


#ifndef KQT_PROC_INIT_UTILS_H
#define KQT_PROC_INIT_UTILS_H


#include <init/devices/Device_impl.h>

#include <stdbool.h>


/**
 * Helper for registering processor key.
 *
 * \param proc_name    The processor name. The Device implementation subclass
 *                     must be defined in local scope with this name.
 * \param field_type   The type name of the field.
 * \param field_name   The name of the field. The caller must define an associated
 *                     function called Proc_ ## proc_name ## _set_ ## field_name.
 * \param keyp         The key pattern -- must not be \c NULL.
 * \param def_value    The default value of the field.
 * \param state_cb     The state modifier callback, or \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
#define REGISTER_SET_WITH_STATE_CB(                                   \
        proc_name, field_type, field_name, keyp, def_value, state_cb) \
    (Device_impl_register_set_ ## field_type(                         \
            &proc_name->parent,                                       \
            keyp,                                                     \
            def_value,                                                \
            Proc_ ## proc_name ## _set_ ## field_name,                \
            state_cb))


/**
 * Helper for registering processor key without state modifier callback.
 *
 * \param proc_name    The processor name. The Device implementation subclass
 *                     must be defined in local scope with this name.
 * \param field_type   The type name of the field.
 * \param field_name   The name of the field. The caller must define an associated
 *                     function called Proc_ ## proc_name ## _set_ ## field_name.
 * \param keyp         The key pattern -- must not be \c NULL.
 * \param def_value    The default value of the field.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
#define REGISTER_SET_FIXED_STATE(proc_name, field_type, field_name, keyp, def_value) \
    (Device_impl_register_set_ ## field_type(                         \
            &proc_name->parent,                                       \
            keyp,                                                     \
            def_value,                                                \
            Proc_ ## proc_name ## _set_ ## field_name,                \
            NULL))


#endif // KQT_PROC_INIT_UTILS_H


