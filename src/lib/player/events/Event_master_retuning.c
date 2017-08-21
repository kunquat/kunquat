

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/events/Event_master_decl.h>

#include <debug/assert.h>
#include <init/Module.h>
#include <init/Tuning_table.h>
#include <kunquat/limits.h>
#include <player/events/Event_common.h>
#include <player/events/Event_params.h>
#include <player/Master_params.h>
#include <player/Tuning_state.h>
#include <Value.h>

#include <stdbool.h>
#include <stdlib.h>


static void get_tuning_info(
        Master_params* master_params,
        Tuning_state** out_tuning_state,
        const Tuning_table** out_tuning_table)
{
    rassert(master_params != NULL);
    rassert(out_tuning_state != NULL);
    rassert(out_tuning_table != NULL);

    const int index = master_params->cur_tuning_state;
    if (index < 0 || index >= KQT_TUNING_TABLES_MAX)
        return;
    Tuning_state* state = master_params->tuning_states[index];
    if (state == NULL)
        return;

    const Module* module = master_params->parent.module;
    const Tuning_table* table = Module_get_tuning_table(module, index);
    if (table == NULL)
        return;

    *out_tuning_state = state;
    *out_tuning_table = table;

    return;
}


bool Event_master_set_retuner_process(
        Master_params* master_params, const Event_params* params)
{
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_INT);

    master_params->cur_tuning_state = (int)params->arg->value.int_type;

    Tuning_state* state = NULL;
    const Tuning_table* table = NULL;
    get_tuning_info(master_params, &state, &table);
    if (state == NULL)
        return true;

    if (!Tuning_state_can_retune(state))
        Tuning_state_reset(state, table);

    return true;
}


bool Event_master_set_retuner_fixed_pitch_process(
        Master_params* master_params, const Event_params* params)
{
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    Tuning_state* state = NULL;
    const Tuning_table* table = NULL;
    get_tuning_info(master_params, &state, &table);
    if (state == NULL)
        return true;

    Tuning_state_set_fixed_pitch(state, table, params->arg->value.float_type);

    return true;
}


bool Event_master_set_retuner_tuning_centre_process(
        Master_params* master_params, const Event_params* params)
{
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    Tuning_state* state = NULL;
    const Tuning_table* table = NULL;
    get_tuning_info(master_params, &state, &table);
    if (state == NULL)
        return true;

    Tuning_state_retune(state, table, params->arg->value.float_type);

    return true;
}


bool Event_master_set_retuner_pitch_offset_process(
        Master_params* master_params, const Event_params* params)
{
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_FLOAT);

    Tuning_state* state = NULL;
    const Tuning_table* table = NULL;
    get_tuning_info(master_params, &state, &table);
    if (state == NULL)
        return true;

    Tuning_state_set_global_offset(state, params->arg->value.float_type);

    return true;
}


bool Event_master_mimic_retuner_process(
        Master_params* master_params, const Event_params* params)
{
    rassert(master_params != NULL);
    rassert(params != NULL);
    rassert(params->arg != NULL);
    rassert(params->arg->type == VALUE_TYPE_INT);

    const int source_index = (int)params->arg->value.int_type;
    if (source_index < 0 || source_index >= KQT_TUNING_TABLES_MAX)
        return true;

    Tuning_state* state = NULL;
    const Tuning_table* table = NULL;
    get_tuning_info(master_params, &state, &table);
    if (state == NULL)
        return true;

    const Module* module = master_params->parent.module;
    const Tuning_table* source = Module_get_tuning_table(module, source_index);

    return Tuning_state_retune_with_source(state, table, source);
}


bool Event_master_reset_retuner_process(
        Master_params* master_params, const Event_params* params)
{
    rassert(master_params != NULL);
    ignore(params);

    Tuning_state* state = NULL;
    const Tuning_table* table = NULL;
    get_tuning_info(master_params, &state, &table);
    if (state == NULL)
        return true;

    Tuning_state_reset(state, table);

    return true;
}


