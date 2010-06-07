

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


#ifndef K_GENERATOR_FIELD_H
#define K_GENERATOR_FIELD_H


#include <stdbool.h>
#include <stdint.h>

#include <File_base.h>
#include <Real.h>
#include <Reltime.h>
//#include <Sample.h>
//#include <Sample_params.h>
#include <Sample_map.h>


typedef enum
{
    GENERATOR_FIELD_NONE = 0,
    GENERATOR_FIELD_BOOL,
    GENERATOR_FIELD_INT,
    GENERATOR_FIELD_FLOAT,
    GENERATOR_FIELD_REAL,
    GENERATOR_FIELD_RELTIME,
    GENERATOR_FIELD_ENVELOPE,
    GENERATOR_FIELD_WAVPACK,
    GENERATOR_FIELD_VORBIS,
    GENERATOR_FIELD_SAMPLE_PARAMS,
    GENERATOR_FIELD_SAMPLE_MAP,
} Generator_field_type;


typedef struct Generator_field Generator_field;


/**
 * Creates a new Generator field.
 *
 * \param key    The key -- must be a valid key. A valid Generator field key
 *               has the suffix .b (boolean), .i (int), .f (float), .r (Real),
 *               .rt (Reltime) or .wv (WavPack).
 * \param data   Pointer to the data that must have a type matching the key,
 *               or \c NULL.
 *
 * \return   The new field if successful, or \c NULL if memory allocation
 *           failed.
 */
Generator_field* new_Generator_field(const char* key, void* data);


/**
 * Creates a new Generator field from data.
 *
 * \param key      The key of the field -- must be a valid Generator field key.
 * \param data     The data -- must not be \c NULL if it has a non-zero
 *                 length.
 * \param length   The length of the data -- must be >= \c 0.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   The new Generator field if successful, otherwise \c NULL.
 *           \a state will _not_ be modified if memory allocation failed.
 */
Generator_field* new_Generator_field_from_data(const char* key,
                                               void* data,
                                               long length,
                                               Read_state* state);


/**
 * Changes the data of a Generator field.
 *
 * This is used for modifying _composition_ data (not playback data).
 *
 * \param field    The Generator field -- must not be \c NULL.
 * \param data     The data -- must not be \c NULL if it has a non-zero
 *                 length.
 * \param length   The length of the data -- must be >= \c 0.
 * \param state    The Read state -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false. \a state will _not_
 *           be modified if memory allocation failed.
 */
bool Generator_field_change(Generator_field* field,
                            void* data,
                            long length,
                            Read_state* state);


/**
 * Compares two Generator fields.
 *
 * \param field1   The first field -- must not be \c NULL.
 * \param field2   The second field -- must not be \c NULL.
 *
 * \return   An integer less than, equal to or greater than zero if \a field1
 *           is found, respectively, to be less than, equal to or greater than
 *           \a field2.
 */
int Generator_field_cmp(const Generator_field* field1,
                        const Generator_field* field2);


/**
 * Sets the Event control bit of the Generator field.
 *
 * If a Generator field has the Event control bit set, it may be empty while
 * still being allocated memory.
 *
 * \param field     The Generator field -- must not be \c NULL. Sample fields
 *                  are not supported.
 * \param control   The Event control flag.
 */
//void Generator_field_set_event_control(Generator_field* field, bool control);


/**
 * Gets the Event control bit of the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL.
 *
 * \return   The Event control flag.
 */
//bool Generator_field_get_event_control(Generator_field* field);


/**
 * Sets the empty flag of the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL. Sample fields
 *                are not supported.
 * \param flag    The empty flag.
 */
void Generator_field_set_empty(Generator_field* field, bool empty);


/**
 * Gets the empty flag of the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL.
 *
 * \return   The empty flag.
 */
bool Generator_field_get_empty(Generator_field* field);


/**
 * Modifies Generator field data.
 *
 * \param field   The Generator field -- must not be \c NULL and must not
 *                contain a Sample.
 * \param str     The new data as a string.
 *
 * \return   \c true if the Generator field was successfully modified,
 *           otherwise \c false.
 */
bool Generator_field_modify(Generator_field* field, char* str);


/**
 * Gets a reference to a boolean value from the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL and must contain
 *                a boolean value.
 *
 * \return   The boolean value.
 */
bool* Generator_field_get_bool(Generator_field* field);


/**
 * Gets a reference to an integer value from the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL and must contain
 *                an integer value.
 *
 * \return   The integer value.
 */
int64_t* Generator_field_get_int(Generator_field* field);


/**
 * Gets a reference to a floating point value from the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL and must contain
 *                a floating point value.
 *
 * \return   The floating point value.
 */
double* Generator_field_get_float(Generator_field* field);


/**
 * Gets a Real value from the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL and must contain
 *                a Real value.
 *
 * \return   The Real value.
 */
Real* Generator_field_get_real(Generator_field* field);


/**
 * Gets a Reltime value from the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL and must contain
 *                a Reltime value.
 *
 * \return   The Reltime value.
 */
Reltime* Generator_field_get_reltime(Generator_field* field);


/**
 * Gets a Sample from the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL and must contain
 *                a Sample.
 *
 * \return   The Sample.
 */
//Sample* Generator_field_get_sample(Generator_field* field);


/**
 * Gets Sample parameters from the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL and must contain
 *                Sample parameters.
 *
 * \return   The Sample parameters.
 */
//Sample_params* Generator_field_get_sample_params(Generator_field* field);


/**
 * Gets a Sample map from the Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL and must contain
 *                a Sample.
 *
 * \return   The Sample map.
 */
Sample_map* Generator_field_get_sample_map(Generator_field* field);


/**
 * Destroys an existing Generator field.
 *
 * \param field   The Generator field -- must not be \c NULL.
 */
void del_Generator_field(Generator_field* field);


#endif // K_GENERATOR_FIELD_H


