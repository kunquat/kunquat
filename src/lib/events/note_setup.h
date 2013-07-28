

/*
 * Author: Tomi Jylhä-Ollila, Finland 2011-2013
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


#include <Instrument.h>
#include <player/Channel.h>


/**
 * Reserves a Voice for a Generator in an Instrument.
 *
 * \param ch         The Channel -- must not be \c NULL.
 * \param ins        The Instrument -- must not be \c NULL.
 * \param gen_num    The number of the Generator -- must be >= \c 0 and
 *                   < \c KQT_GENERATORS_MAX.
 */
void reserve_voice(Channel* ch, Instrument* ins, int gen_num);


/**
 * Sets initial values of parameters according to the Instrument.
 *
 * \param voice       The Voice -- must not be \c NULL.
 * \param vs          The Voice state -- must not be \c NULL.
 * \param ch          The Channel -- must not be \c NULL.
 * \param force_var   A reference to the force variation value -- must not be
 *                    \c NULL.
 */
void set_instrument_properties(
        Voice* voice,
        Voice_state* vs,
        Channel* ch,
        double* force_var);


#endif // K_NOTE_SETUP_H


