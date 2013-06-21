

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2012
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
#define KQT_BUFFER_SIZE_MAX 1048576


/**
 * Maximum calculated length of a Kunquat composition.
 *
 * The constant corresponds to 30 days.
 */
#define KQT_MAX_CALC_DURATION 2592000000000000LL


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
 * Maximum number of Patterns in a Kunquat Handle.
 */
#define KQT_PATTERNS_MAX 1024


/**
 * Maximum number of Pattern instances in a Pattern.
 */
#define KQT_PAT_INSTANCES_MAX KQT_PATTERNS_MAX


/**
 * Maximum number of Voice Columns in a Kunquat Handle (Global Column not
 * included).
 */
#define KQT_COLUMNS_MAX 64


/**
 * This specifies how many parts one beat is. The prime factorisation of this
 * number is (2^7)*(3^4)*5*7*11*13*17.
 */
#define KQT_TSTAMP_BEAT (882161280L)


/**
 * Maximum number of Instruments in a Kunquat Handle.
 */
#define KQT_INSTRUMENTS_MAX 256


/**
 * Maximum number of Generators in a Kunquat Instruemnt.
 */
#define KQT_GENERATORS_MAX 256


/**
 * Maximum number of Effects in the global Connections.
 */
#define KQT_EFFECTS_MAX 256


/**
 * Maximum number of Effects in a Kunquat instrument.
 */
#define KQT_INST_EFFECTS_MAX 16


/**
 * Maximum number of DSPs inside an Effect.
 */
#define KQT_DSPS_MAX 64


/**
 * Maximum number of input/output ports in a Device.
 */
#define KQT_DEVICE_PORTS_MAX KQT_GENERATORS_MAX


/**
 * Maximum number of distinct hit values in a Kunquat instrument.
 */
#define KQT_HITS_MAX 128


/**
 * Maximum number of Scales in a Kunquat Handle.
 */
#define KQT_SCALES_MAX 16

#define KQT_SCALE_OCTAVES 16
#define KQT_SCALE_MIDDLE_OCTAVE_UNBIASED 8
#define KQT_SCALE_NOTE_MODS 16
#define KQT_SCALE_NOTES 128
#define KQT_SCALE_OCTAVE_BIAS (-3)
#define KQT_SCALE_OCTAVE_FIRST KQT_SCALE_OCTAVE_BIAS
#define KQT_SCALE_MIDDLE_OCTAVE (KQT_SCALE_MIDDLE_OCTAVE_UNBIASED + KQT_SCALE_OCTAVE_BIAS)
#define KQT_SCALE_OCTAVE_LAST (KQT_SCALE_OCTAVES - 1 + KQT_SCALE_OCTAVE_BIAS)


/**
 * Maximum number of notes in the arpeggio.
 */
#define KQT_ARPEGGIO_NOTES_MAX 64


#ifdef __cplusplus
}
#endif


#endif // KQT_LIMITS_H


