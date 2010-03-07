

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

#include <Generator_type.h>
#include <Instrument_params.h>
#include <Random.h>
#include <Voice_state.h>
#include <File_base.h>


/**
 * Generator is an object used for creating sound based on a specific sound
 * synthesising method.
 */
typedef struct Generator
{
    Gen_type type;
    bool enabled;
    double volume_dB;
    double volume;
    Random* random;
    bool (*parse)(struct Generator*, const char*, void*, long, Read_state*);
    void (*init_state)(struct Generator*, Voice_state*);
    void (*destroy)(struct Generator*);
    uint32_t (*mix)(struct Generator*, Voice_state*, uint32_t, uint32_t, uint32_t, double,
                int, kqt_frame**);
    Instrument_params* ins_params;
} Generator;


#define GENERATOR_DEFAULT_ENABLED (false)
#define GENERATOR_DEFAULT_VOLUME (0)


/**
 * Creates a new Generator of the specified type.
 *
 * \param type         The Generator type -- must be a valid and supported
 *                     type.
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 *
 * \return   The new Generator if successful, or \c NULL if memory allocation
 *           failed.
 */
Generator* new_Generator(Gen_type type, Instrument_params* ins_params);


/**
 * Initialises the general Generator parameters.
 *
 * \param gen      The Generator -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Generator_init(Generator* gen);


/**
 * Uninitialises the general Generator parameters.
 *
 * \param gen   The Generator -- must not be \c NULL.
 */
void Generator_uninit(Generator* gen);


/**
 * Copies the general Generator parameters.
 *
 * \param dest   The destination Generator -- must not be \c NULL.
 * \param src    The source Generator -- must not be \c NULL.
 */
void Generator_copy_general(Generator* dest, Generator* src);


/**
 * Parses general Generator header (p_generator.json).
 *
 * \param gen     The Generator -- must not be \c NULL.
 * \param str     The textual description.
 * \param state   The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. The state error will
 *           _not_ be set in case memory allocation failed.
 */
bool Generator_parse_general(Generator* gen, char* str, Read_state* state);


/**
 * Parses data associated with the Generator.
 *
 * \param gen      The Generator -- must not be \c NULL.
 * \param subkey   The subkey. This is the part after "generator_XX/".
 *                 \a subkey must be part of the type specification of
 *                 \a gen.
 * \param data     The data -- must not be \c NULL unless \a length is 0.
 * \param length   The length of the data -- must be >= \c 0.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. The Read state error
 *           will _not_ be set if memory allocation failed.
 */
bool Generator_parse(Generator* gen,
                     const char* subkey,
                     void* data,
                     long length,
                     Read_state* state);


/**
 * Returns the type of the Generator.
 *
 * \param gen   The Generator -- must not be \c NULL.
 *
 * \return   The type.
 */
Gen_type Generator_get_type(Generator* gen);


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


