

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010
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

#include <Device.h>
#include <Device_params.h>
#include <Gen_conf.h>
#include <Instrument_params.h>
#include <kunquat/limits.h>
#include <pitch_t.h>
#include <Random.h>
#include <Voice_state.h>
#include <File_base.h>


#define GEN_TYPE_LENGTH_MAX 128


/**
 * Generator is an object used for creating sound based on a specific sound
 * synthesising method.
 */
typedef struct Generator
{
    Device parent;
    char type[GEN_TYPE_LENGTH_MAX];
    Random* random;
    Gen_conf* conf;
    void (*init_state)(struct Generator*, Voice_state*);
    void (*destroy)(struct Generator*);
    uint32_t (*mix)(struct Generator*, Voice_state*, uint32_t, uint32_t,
                    uint32_t, double);
    Instrument_params* ins_params;
} Generator;


/**
 * Creates a new Generator of the specified type.
 *
 * \param str           A textual representation of the Generator type -- must
 *                      not be \c NULL.
 * \param ins_params    The Instrument parameters -- must not be \c NULL.
 * \param buffer_size   The mixing buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 * \param random        The Random source -- must not be \c NULL.
 * \param state         The Read state -- must not be \c NULL.
 *
 * \return   The new Generator if successful, or \c NULL if memory allocation
 *           failed.
 */
Generator* new_Generator(char* str,
                         Instrument_params* ins_params,
                         uint32_t buffer_size,
                         uint32_t mix_rate,
                         Random* random,
                         Read_state* state);


/**
 * Initialises the general Generator parameters.
 *
 * \param gen           The Generator -- must not be \c NULL.
 * \param destroy       The destructor of the Generator --
 *                      must not be \c NULL.
 * \param mix           The mixing function of the Generator --
 *                      must not be \c NULL.
 * \param init_state    The Voice state initialiser, or \c NULL if not needed.
 * \param buffer_size   The buffer size -- must be > \c 0 and
 *                      <= \c KQT_BUFFER_SIZE_MAX.
 * \param mix_rate      The mixing rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Generator_init(Generator* gen,
                    void (*destroy)(Generator*),
                    uint32_t (*mix)(Generator*, Voice_state*, uint32_t, uint32_t, uint32_t, double),
                    void (*init_state)(Generator*, Voice_state*),
                    uint32_t buffer_size,
                    uint32_t mix_rate);


/**
 * Sets the configuration of the Generator.
 *
 * \param gen    The Generator -- must not be \c NULL.
 * \param conf   The Generator configuration -- must not be \c NULL.
 */
void Generator_set_conf(Generator* gen, Gen_conf* conf);


/**
 * Retrieves the Generator parameter tree.
 *
 * \param gen   The Generator -- must not be \c NULL.
 *
 * \return   The Generator parameter tree.
 */
Device_params* Generator_get_params(Generator* gen);


/**
 * Returns the type of the Generator.
 *
 * \param gen   The Generator -- must not be \c NULL.
 *
 * \return   The type.
 */
char* Generator_get_type(Generator* gen);


/**
 * Handles a given note as appropriate for the Generator.
 *
 * \param gen      The Generator -- must not be \c NULL.
 * \param states   The array of Voice states -- must not be \c NULL.
 * \param cents    The pitch in cents -- must be finite.
 */
void Generator_process_note(Generator* gen,
                            Voice_state* states,
                            double cents);


/**
 * Mixes the Generator.
 *
 * \param gen       The Generator -- must not be \c NULL.
 * \param state     The Voice state -- must not be \c NULL.
 * \param nframes   The number of frames to be mixed -- must not be greater
 *                  than the mixing buffer size.
 * \param offset    The starting frame offset (\a nframes - \a offset are
 *                  actually mixed).
 * \param freq      The mixing frequency -- must be > \c 0.
 * \param tempo     The current tempo -- must be > \c 0.
 */
void Generator_mix(Generator* gen,
                   Voice_state* state,
                   uint32_t nframes,
                   uint32_t offset,
                   uint32_t freq,
                   double tempo);


/**
 * Uninitialises an existing Generator.
 *
 * \param gen   The Generator -- must not be \c NULL.
 */
void del_Generator(Generator* gen);


#endif // K_GENERATOR_H


