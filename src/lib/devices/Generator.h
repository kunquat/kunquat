

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


typedef uint32_t Generator_process_vstate_func(
        const Generator*,
        Gen_state*,
        Ins_state*,
        Voice_state*,
        const Work_buffers*,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
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
    Generator_process_vstate_func* process_vstate;
    void (*clear_history)(const Device_impl*, Gen_state*);
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
 * \param gen              The Generator -- must not be \c NULL.
 * \param destroy          The destructor of the Generator --
 *                         must not be \c NULL.
 * \param process_vstate   The Voice state process function of the Generator,
 *                         or \c NULL if not needed.
 * \param init_vstate      The Voice state initialiser, or \c NULL if not needed.
 * \param process_signal   The signal processing function, or \c NULL if not needed.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Generator_init(
        Generator* gen,
        //void (*destroy)(Generator*),
        Generator_process_vstate_func process_vstate,
        void (*init_vstate)(const Generator*, const Gen_state*, Voice_state*),
        Device_process_signal_func* process_signal);


/**
 * Set a function that clears the internal buffers of the Generator
 * implementation.
 *
 * \param gen    The Generator -- must not be \c NULL.
 * \param func   The clear function -- must not be \c NULL.
 */
void Generator_set_clear_history(
        Generator* gen, void(*func)(const Device_impl*, Gen_state*));


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
 * Clear the internal buffers (if any) of the Generator.
 *
 * \param gen         The Generator -- must not be \c NULL.
 * \param gen_state   The Generator state -- must not be \c NULL.
 */
void Generator_clear_history(const Generator* gen, Gen_state* gen_state);


/**
 * Return the type of the Generator.
 *
 * \param gen   The Generator -- must not be \c NULL.
 *
 * \return   The type.
 */
//const char* Generator_get_type(const Generator* gen);


/**
 * Process a Voice state that belongs to the Generator.
 *
 * \param gen          The Generator -- must not be \c NULL.
 * \param dstates      The Device states -- must not be \c NULL.
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param buf_start    The start index of the buffer area to be processed.
 * \param buf_stop     The stop index of the buffer area to be processed.
 * \param audio_rate   The mixing frequency -- must be > \c 0.
 * \param tempo        The current tempo -- must be > \c 0.
 */
void Generator_process_vstate(
        const Generator* gen,
        Device_states* dstates,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo);


/**
 * Destroy an existing Generator.
 *
 * \param gen   The Generator, or \c NULL.
 */
void del_Generator(Generator* gen);


#endif // K_GENERATOR_H


