

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2016
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DEVICE_FIELD_H
#define KQT_DEVICE_FIELD_H


#include <init/devices/param_types/Envelope.h>
#include <init/devices/param_types/Hit_map.h>
#include <init/devices/param_types/Note_map.h>
#include <init/devices/param_types/Num_list.h>
#include <init/devices/param_types/Padsynth_params.h>
#include <init/devices/param_types/Sample.h>
#include <init/devices/param_types/Sample_params.h>
#include <mathnum/Tstamp.h>
#include <string/Streader.h>

#include <stdbool.h>
#include <stdint.h>


typedef enum
{
    DEVICE_FIELD_NONE = 0,
    DEVICE_FIELD_BOOL,
    DEVICE_FIELD_INT,
    DEVICE_FIELD_FLOAT,
    DEVICE_FIELD_TSTAMP,
    DEVICE_FIELD_ENVELOPE,
    DEVICE_FIELD_WAVPACK,
    DEVICE_FIELD_WAV,
    DEVICE_FIELD_VORBIS,
    DEVICE_FIELD_SAMPLE_PARAMS,
    DEVICE_FIELD_NOTE_MAP,
    DEVICE_FIELD_HIT_MAP,
    DEVICE_FIELD_NUM_LIST,
    DEVICE_FIELD_PADSYNTH_PARAMS,

    DEVICE_FIELD_COUNT_
} Device_field_type;


/**
 * Get the Device field type of a key pattern.
 *
 * \param keyp   The key pattern -- must not be \c NULL.
 *
 * \return   The Device field type, or \c DEVICE_FIELD_NONE if the key pattern
 *           does not contain proper type information.
 */
Device_field_type get_keyp_device_field_type(const char* keyp);


typedef struct Device_field Device_field;


/**
 * Create a new Device field.
 *
 * \param key    The key -- must be a valid Device field key.
 * \param data   Pointer to the data that must have a type matching the key,
 *               or \c NULL.
 *
 * \return   The new field if successful, or \c NULL if memory allocation
 *           failed.
 */
Device_field* new_Device_field(const char* key, void* data);


/**
 * Create a new Device field from data.
 *
 * \param key   The key of the field -- must be a valid Device field key.
 * \param sr    The Streader of the data -- must not be \c NULL.
 *
 * \return   The new Device field if successful, otherwise \c NULL.
 */
Device_field* new_Device_field_from_data(const char* key, Streader* sr);


/**
 * Return the key of the Device field.
 *
 * \param field   The Device field -- must not be \c NULL.
 *
 * \return   The key of the Device field.
 */
const char* Device_field_get_key(const Device_field* field);


/**
 * Change the data of a Device field.
 *
 * This is used for modifying _composition_ data (not playback data).
 *
 * \param field   The Device field -- must not be \c NULL.
 * \param sr      The Streader of the data -- must not be \c NULL.
 *
 * \return   \c true if successful, otherwise \c false.
 */
bool Device_field_change(Device_field* field, Streader* sr);


/**
 * Compare two Device fields.
 *
 * \param field1   The first field -- must not be \c NULL.
 * \param field2   The second field -- must not be \c NULL.
 *
 * \return   An integer less than, equal to or greater than zero if \a field1
 *           is found, respectively, to be less than, equal to or greater than
 *           \a field2.
 */
int Device_field_cmp(const Device_field* field1, const Device_field* field2);


/**
 * Set the empty flag of the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must have
 *                real-time type.
 * \param empty   The empty flag.
 */
void Device_field_set_empty(Device_field* field, bool empty);


/**
 * Get the empty flag of the Device field.
 *
 * \param field   The Device field -- must not be \c NULL.
 *
 * \return   The empty flag.
 */
bool Device_field_get_empty(const Device_field* field);


/**
 * Modify Device field data.
 *
 * \param field   The Device field -- must not be \c NULL and must
 *                contain a value that is modifiable in real time.
 * \param str     The new data -- must not be \c NULL.
 *
 * \return   \c true if the Device field was successfully modified,
 *           otherwise \c false.
 */
//bool Device_field_modify(Device_field* field, void* data);


/**
 * Get a reference to a boolean value from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                a boolean value.
 *
 * \return   The boolean value.
 */
const bool* Device_field_get_bool(const Device_field* field);


/**
 * Get a reference to an integer value from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                an integer value.
 *
 * \return   The integer value.
 */
const int64_t* Device_field_get_int(const Device_field* field);


/**
 * Get a reference to a floating point value from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                a floating point value.
 *
 * \return   The floating point value.
 */
const double* Device_field_get_float(const Device_field* field);


/**
 * Get a Tstamp value from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                a Tstamp value.
 *
 * \return   The Tstamp value.
 */
const Tstamp* Device_field_get_tstamp(const Device_field* field);


/**
 * Get an Envelope value from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                an Envelope value.
 *
 * \return   The Envelope value.
 */
const Envelope* Device_field_get_envelope(const Device_field* field);


/**
 * Get a Sample from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                a Sample.
 *
 * \return   The Sample.
 */
const Sample* Device_field_get_sample(const Device_field* field);


/**
 * Get Sample parameters from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                Sample parameters.
 *
 * \return   The Sample parameters.
 */
const Sample_params* Device_field_get_sample_params(const Device_field* field);


/**
 * Get a Note map from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                a Note map.
 *
 * \return   The Note map.
 */
const Note_map* Device_field_get_note_map(const Device_field* field);


/**
 * Get a Hit map from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                a Hit map.
 *
 * \return   The Hit map.
 */
const Hit_map* Device_field_get_hit_map(const Device_field* field);


/**
 * Get a Number list from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                a Number list.
 *
 * \return   The Number list.
 */
const Num_list* Device_field_get_num_list(const Device_field* field);


/**
 * Get PADsynth parameters from the Device field.
 *
 * \param field   The Device field -- must not be \c NULL and must contain
 *                PADsynth parameters.
 *
 * \return   The PADsynth parameters.
 */
const Padsynth_params* Device_field_get_padsynth_params(const Device_field* field);


/**
 * Destroy an existing Device field.
 *
 * \param field   The Device field, or \c NULL.
 */
void del_Device_field(Device_field* field);


#endif // KQT_DEVICE_FIELD_H


