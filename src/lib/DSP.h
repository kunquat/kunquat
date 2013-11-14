

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2013
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_DSP_H
#define K_DSP_H


#include <stdint.h>

#include <Device.h>
#include <player/DSP_state.h>


#define DSP_TYPE_LENGTH_MAX 128


/**
 * DSP is a Digital Signal Processing effect.
 */
typedef struct DSP
{
    Device parent;
    //char type[DSP_TYPE_LENGTH_MAX];

    void (*clear_history)(const Device_impl*, DSP_state*);
} DSP;


/**
 * Creates a new DSP.
 *
 * \return   The new DSP if successful, otherwise \c NULL. \a state will not
 *           be modified if memory allocation failed.
 */
DSP* new_DSP();


/**
 * Initialises the common part of the DSP.
 *
 * \param dsp           The DSP -- must not be \c NULL.
 * \param destroy       The destructor of the DSP -- must not be \c NULL.
 * \param process       The Device process function -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool DSP_init(
        DSP* dsp,
        void (*process)(
            Device*,
            Device_states*,
            uint32_t,
            uint32_t,
            uint32_t,
            double));


/**
 * Sets a function that clears the internal buffers of the DSP implementation.
 *
 * \param dsp    The DSP -- must not be \c NULL.
 * \param func   The clear function -- must not be \c NULL.
 */
void DSP_set_clear_history(
        DSP* dsp, void (*func)(const Device_impl*, DSP_state*));


/**
 * Resets the playback parameters of the DSP.
 *
 * If you override this function, call this inside the overriding function.
 *
 * \param dsp   The DSP Device -- must not be \c NULL.
 */
//void DSP_reset(Device* device, Device_states* dstates);


/**
 * Clears the internal buffers (if any) of the DSP.
 *
 * \param dsp         The DSP -- must not be \c NULL.
 * \param dsp_state   The DSP state -- must not be \c NULL.
 */
void DSP_clear_history(const DSP* dsp, DSP_state* dsp_state);


/**
 * Destroys an existing DSP.
 *
 * \param dsp   The DSP, or \c NULL.
 */
void del_DSP(DSP* dsp);


#endif // K_DSP_H


