

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


#ifndef K_NOTE_SETUP_H
#define K_NOTE_SETUP_H


#include <devices/Audio_unit.h>
#include <player/Channel.h>


/**
 * Reserve a Voice for a processor in an Audio unit.
 *
 * \param ch           The Channel -- must not be \c NULL.
 * \param au           The Audio unit -- must not be \c NULL.
 * \param proc_state   The processor state -- must not be \c NULL.
 * \param proc_num     The number of the Processor -- must be >= \c 0 and
 *                     < \c KQT_PROCESSORS_MAX.
 */
void reserve_voice(
        Channel* ch, Audio_unit* au, const Proc_state* proc_state, int proc_num);


/**
 * Set initial values of parameters according to the Audio unit.
 *
 * \param voice       The Voice -- must not be \c NULL.
 * \param vs          The Voice state -- must not be \c NULL.
 * \param ch          The Channel -- must not be \c NULL.
 * \param force_var   A reference to the force variation value -- must not be
 *                    \c NULL.
 */
void set_au_properties(Voice* voice, Voice_state* vs, Channel* ch, double* force_var);


#endif // K_NOTE_SETUP_H


