

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2015
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef K_GENERATOR_H
#define K_GENERATOR_H


#include <stdbool.h>
#include <stdint.h>

#include <devices/Device.h>
#include <devices/Device_params.h>
#include <devices/Instrument_params.h>
#include <kunquat/limits.h>
#include <pitch_t.h>
#include <player/Gen_state.h>
#include <player/Ins_state.h>
#include <player/Voice_state.h>
#include <player/Work_buffers.h>


#define GEN_TYPE_LENGTH_MAX 128


/**
 * Generator is an object used for creating sound based on a specific sound
 * synthesising method.
 */
typedef struct Generator Generator;


typedef uint32_t Generator_mix_func(
        const Generator*,
        Gen_state*,
        Ins_state*,
        Voice_state*,
        const Work_buffers*,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo);


struct Generator
{
    Device parent;
    //char type[GEN_TYPE_LENGTH_MAX];
    const Instrument_params* ins_params;

    void (*init_vstate)(
            const struct Generator*,
            const Gen_state*,
            Voice_state*);
    Generator_mix_func* mix;
};


/**
 * Create a new Generator of the specified type.
 *
 * \param ins_params    The Instrument parameters -- must not be \c NULL.
 *
 * \return   The new Generator if successful, or \c NULL if memory allocation
 *           failed.
 */
Generator* new_Generator(const Instrument_params* ins_params);


/**
 * Initialise the general Generator parameters.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param destroy       The destructor of the Generator --
 *                      must not be \c NULL.
 * \param mix           The mixing function of the Generator --
 *                      must not be \c NULL.
 * \param init_state    The Voice state initialiser, or \c NULL if not needed.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Generator_init(
        Generator* gen,
        //void (*destroy)(Generator*),
        Generator_mix_func mix,
        void (*init_vstate)(const Generator*, const Gen_state*, Voice_state*));


/**
 * Reset the playback parameters of the Generator.
 *
 * If you override this function, call this inside the overriding function.
 *
 * \param gen       The Generator Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 */
//void Generator_reset(Device* device, Device_states* dstates);


/**
 * Return the type of the Generator.
 *
 * \param gen   The Generator -- must not be \c NULL.
 *
 * \return   The type.
 */
//const char* Generator_get_type(const Generator* gen);


/**
 * Mix the Generator.
 *
 * \param gen       The Generator -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param vstate    The Voice state -- must not be \c NULL.
 * \param wbs       The Work buffers -- must not be \c NULL.
 * \param nframes   The number of frames to be mixed -- must not be greater
 *                  than the mixing buffer size.
 * \param offset    The starting frame offset (\a nframes - \a offset are
 *                  actually mixed).
 * \param freq      The mixing frequency -- must be > \c 0.
 * \param tempo     The current tempo -- must be > \c 0.
 */
void Generator_mix(
        const Generator* gen,
        Device_states* dstates,
        Voice_state* vstate,
        const Work_buffers* wbs,
        uint32_t nframes,
        uint32_t offset,
        uint32_t freq,
        double tempo);


/**
 * Destroy an existing Generator.
 *
 * \param gen   The Generator, or \c NULL.
 */
void del_Generator(Generator* gen);


#endif // K_GENERATOR_H


