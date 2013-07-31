

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
#include <DSP_conf.h>
#include <File_base.h>
#include <player/DSP_state.h>


#define DSP_TYPE_LENGTH_MAX 128


/**
 * DSP is a Digital Signal Processing effect.
 */
typedef struct DSP
{
    Device parent;
    char type[DSP_TYPE_LENGTH_MAX];
    DSP_conf* conf;

    void (*clear_history)(struct DSP*);
    void (*destroy)(struct DSP*);
} DSP;


/**
 * Creates a new DSP.
 *
 * \param str           A textual representation of the DSP type -- must not
 *                      be \c NULL.
 * \param buffer_size   The buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 * \param state         The Read state -- must not be \c NULL.
 *
 * \return   The new DSP if successful, otherwise \c NULL. \a state will not
 *           be modified if memory allocation failed.
 */
DSP* new_DSP(
        char* str,
        uint32_t buffer_size,
        uint32_t mix_rate,
        Read_state* state);


/**
 * Initialises the common part of the DSP.
 *
 * \param dsp           The DSP -- must not be \c NULL.
 * \param destroy       The destructor of the DSP -- must not be \c NULL.
 * \param process       The Device process function -- must not be \c NULL.
 * \param buffer_size   The buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool DSP_init(
        DSP* dsp,
        void (*destroy)(DSP*),
        void (*process)(
            Device*,
            Device_states*,
            uint32_t,
            uint32_t,
            uint32_t,
            double),
        uint32_t buffer_size,
        uint32_t mix_rate);


/**
 * Sets a function that clears the internal buffers of the DSP.
 *
 * \param dsp    The DSP -- must not be \c NULL.
 * \param func   The function -- must not be \c NULL.
 */
void DSP_set_clear_history(DSP* dsp, void (*func)(DSP*));


/**
 * Resets the playback parameters of the DSP.
 *
 * If you override this function, call this inside the overriding function.
 *
 * \param dsp   The DSP Device -- must not be \c NULL.
 */
void DSP_reset(Device* device, Device_states* dstates);


/**
 * Clears the internal buffers (if any) of the DSP.
 *
 * \param dsp   The DSP -- must not be \c NULL.
 */
void DSP_clear_history(DSP* dsp);


/**
 * Sets the configuration of the DSP.
 *
 * \param dsp    The DSP -- must not be \c NULL.
 * \param conf   The DSP configuration -- must not be \c NULL.
 */
void DSP_set_conf(DSP* dsp, DSP_conf* conf);


/**
 * Destroys an existing DSP.
 *
 * \param dsp   The DSP, or \c NULL.
 */
void del_DSP(DSP* dsp);


#endif // K_DSP_H


