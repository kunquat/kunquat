

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_TRIGGERS_H
#define K_TRIGGERS_H


#include <stdbool.h>
#include <stdlib.h>

#include <Environment.h>
#include <Event_names.h>
#include <Event_type.h>
#include <File_base.h>
#include <Random.h>
#include <Value.h>


/**
 * Extracts event type information from a trigger/event description.
 *
 * \param desc       The trigger/event description -- must not be \c NULL.
 * \param names      The Event names -- must not be \c NULL.
 * \param rs         The Read state -- must not be \c NULL.
 * \param ret_name   Address where the event name will be stored -- must
 *                   not be \c NULL and must have space for at least
 *                   \c EVENT_NAME_MAX characters.
 * \param ret_type   Address where the event type will be stored -- must
 *                   not be \c NULL.
 *
 * \return   The position of \a desc after parsing.
 */
char* get_event_type_info(
        char* desc,
        const Event_names* names,
        Read_state* rs,
        char* ret_name,
        Event_type* ret_type);


/**
 * Processes trigger expression.
 *
 * \param arg_expr     The argument expression -- must not be \c NULL.
 * \param field_type   The field type required -- must be valid.
 * \param env          The Environment -- must not be \c NULL.
 * \param random       The Random source -- must not be \c NULL.
 * \param meta         The meta variable, or \c NULL.
 * \param rs           The Read state -- must not be \c NULL.
 * \param ret_arg      Address where the event argument will be stored --
 *                     must not be \c NULL.
 *
 * \return   The position of \a arg_expr after parsing.
 */
char* process_expr(
        char* arg_expr,
        Value_type field_type,
        Environment* env,
        Random* random,
        const Value* meta,
        Read_state* rs,
        Value* ret_value);


/**
 * Extracts event properties from a trigger.
 *
 * \param trigger_desc   The trigger description -- must not be \c NULL.
 * \param names          The Event names -- must not be \c NULL.
 * \param env            The Environment -- must not be \c NULL.
 * \param random         The Random source -- must not be \c NULL.
 * \param meta           The meta variable, or \c NULL.
 * \param rs             The Read state -- must not be \c NULL.
 * \param ret_name       Address where the event name will be stored -- must
 *                       not be \c NULL and must have space for at least
 *                       \c EVENT_NAME_MAX characters.
 * \param ret_type       Address where the event type will be stored -- must
 *                       not be \c NULL.
 * \param ret_arg        Address where the event argument will be stored --
 *                       must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if parsing failed.
 */
bool process_trigger(
        char* trigger_desc,
        const Event_names* names,
        Environment* env,
        Random* random,
        const Value* meta,
        Read_state* rs,
        char* ret_name,
        Event_type* ret_type,
        Value* ret_value);


#endif // K_TRIGGERS_H


