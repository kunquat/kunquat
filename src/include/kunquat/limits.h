

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_LIMITS_H
#define KQT_LIMITS_H


#ifdef __cplusplus
extern "C" {
#endif


/**
 * The current file format version used by Kunquat.
 */
#define KQT_FORMAT_VERSION "00"


/**
 * Maximum number of Kunquat Handles that can be open simultaneously.
 */
#define KQT_HANDLES_MAX 256


/**
 * Maximum length of a key inside a Kunquat composition.
 */
#define KQT_KEY_LENGTH_MAX 73


/**
 * Maximum number of output buffers a Kunquat Handle can contain.
 */
#define KQT_BUFFERS_MAX 2


/**
 * Maximum size of an output buffer in frames.
 *
 * This upper limit is a safety measure. Typical applications use much smaller
 * buffer sizes, e.g. 1024 to 8192 frames.
 */
#define KQT_AUDIO_BUFFER_SIZE_MAX 1048576


/**
 * Maximum number of threads supported for audio rendering.
 */
#define KQT_THREADS_MAX 32


/**
 * Maximum calculated length of a Kunquat composition.
 *
 * The constant corresponds to 30 days.
 */
#define KQT_CALC_DURATION_MAX 2592000000000000LL


/**
 * Maximum number of Voices used for mixing.
 */
#define KQT_VOICES_MAX 1024


/**
 * Maximum number of songs in a Kunquat Handle.
 */
#define KQT_SONGS_MAX 256


/**
 * Maximum number of tracks in a Kunquat Handle.
 */
#define KQT_TRACKS_MAX KQT_SONGS_MAX


/**
 * Maximum number of systems in a Kunquat Song.
 */
#define KQT_SYSTEMS_MAX 1024


/**
 * Maximum number of Patterns in a Kunquat Handle.
 */
#define KQT_PATTERNS_MAX 1024


/**
 * Maximum number of Pattern instances in a Pattern.
 */
#define KQT_PAT_INSTANCES_MAX KQT_PATTERNS_MAX


/**
 * Maximum number of Columns in a Kunquat Handle.
 */
#define KQT_COLUMNS_MAX 64


/**
 * Maximum number of channels in a Kunquat Handle.
 */
#define KQT_CHANNELS_MAX KQT_COLUMNS_MAX


/**
 * This specifies how many parts one beat is. The prime factorisation of this
 * number is (2^7)*(3^4)*5*7*11*13*17.
 */
#define KQT_TSTAMP_BEAT (882161280L)


/**
 * Tempo slides are performed in fixed slices in composition time. This is the
 * length of one slice in beat remainders (the final slice in a tempo slice
 * may be shorter).
 */
#define KQT_TEMPO_SLIDE_SLICE_LEN (KQT_TSTAMP_BEAT / 24)


/**
 * Maximum number of Audio units in a Kunquat Handle.
 */
#define KQT_AUDIO_UNITS_MAX 256


/**
 * Maximum number of Audio unit controls in a Kunquat Handle.
 */
#define KQT_CONTROLS_MAX KQT_AUDIO_UNITS_MAX


/**
 * Maximum number of processors in a Kunquat instrument or effect.
 */
#define KQT_PROCESSORS_MAX 256


/**
 * Maximum number of input/output ports in a Device.
 */
#define KQT_DEVICE_PORTS_MAX KQT_PROCESSORS_MAX


/**
 * Maximum number of distinct hit values in an audio unit.
 */
#define KQT_HITS_MAX 128


/**
 * Maximum number of Tuning tables in a Kunquat Handle.
 */
#define KQT_TUNING_TABLES_MAX 16

#define KQT_TUNING_TABLE_OCTAVES 16
#define KQT_TUNING_TABLE_NOTES_MAX 128


/**
 * Maximum length of an event name (not including null terminator).
 */
#define KQT_EVENT_NAME_MAX 12


/**
 * Maximum length of a variable name (not including null terminator).
 */
#define KQT_VAR_NAME_MAX 32

/**
 * Allowed characters in variable names.
 */
#define KQT_VAR_INIT_CHARS "abcdefghijklmnopqrstuvwxyz_"
#define KQT_VAR_CHARS KQT_VAR_INIT_CHARS "0123456789"


/**
 * Maximum length of a device event name (not including null terminator).
 */
#define KQT_DEVICE_EVENT_NAME_MAX 32

/**
 * Allowed characters in device event names.
 */
#define KQT_DEVICE_EVENT_CHARS "abcdefghijklmnopqrstuvwxyz_0123456789"


/**
 * Maximum length of a trigger name (including null terminator).
 */
#define KQT_TRIGGER_NAME_MAX (KQT_EVENT_NAME_MAX + 1 + KQT_DEVICE_EVENT_NAME_MAX)


#ifdef __cplusplus
}
#endif


#endif // KQT_LIMITS_H


