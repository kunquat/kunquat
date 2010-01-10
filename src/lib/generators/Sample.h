

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


#ifndef K_SAMPLE_H
#define K_SAMPLE_H


#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include <kunquat/frame.h>
#include <Voice_state.h>
#include <Generator.h>


typedef enum
{
    /// Uninitialised.
    SAMPLE_FORMAT_NONE = 0,
    /// WavPack.
    SAMPLE_FORMAT_WAVPACK,
    /// Vorbis.
    // SAMPLE_FORMAT_VORBIS,
    /// Sentinel -- not a valid format.
    SAMPLE_FORMAT_LAST
} Sample_format;


typedef enum
{
    SAMPLE_LOOP_OFF = 0, ///< No loop.
    SAMPLE_LOOP_UNI,     ///< Unidirectional (forward) loop.
    SAMPLE_LOOP_BI       ///< Bidirectional loop.
} Sample_loop;


/**
 * Common parameters for a Sample (the parameters stored in p_sample.json).
 */
typedef struct Sample_params
{
    Sample_format format; ///< The file format.
    double mid_freq;      ///< The playback frequency used to represent 440 Hz tone.
    Sample_loop loop;     ///< Loop setting.
    uint64_t loop_start;  ///< Loop start.
    uint64_t loop_end;    ///< Loop end (the frame at this index will not be played).
} Sample_params;


/**
 * Sample contains a digital sound sample.
 */
typedef struct Sample
{
    Sample_params params;
    char* path;           ///< The path of the file (if applicable -- otherwise NULL).
    bool changed;         ///< Whether the sample (sound) data has changed after loading.
    bool is_lossy;        ///< Whether this sample uses lossy compression.
    int channels;         ///< The number of channels (1 or 2).
    int bits;             ///< The bit resolution (8, 16, 24 or 32).
    bool is_float;        ///< Whether this sample is in floating point format.
    uint64_t len;         ///< The length of the sample (in amplitude values per channel).
    void* data[2];        ///< The sample data.
} Sample;


/**
 * Initialises common Sample parameters.
 *
 * \param params   The Sample parameters -- must not be \c NULL.
 *
 * \return   The parameter \a params.
 */
Sample_params* Sample_params_init(Sample_params* params);


/**
 * Copies Sample parameters.
 *
 * \param dest   The copy destination -- must not be \c NULL.
 * \param src    The copy source -- must not be \c NULL.
 *
 * \return   The parameter \a dest.
 */
Sample_params* Sample_params_copy(Sample_params* dest, Sample_params* src);


/**
 * Copies Sample parameters into a Sample.
 *
 * This function copies all the fields except the format field.
 *
 * \param sample   The destination Sample -- must not be \c NULL.
 * \param params   The Sample parameters -- must not be \c NULL.
 */
void Sample_set_params(Sample* sample, Sample_params* params);


/**
 * Creates a new Sample.
 *
 * \return   The new Sample if successful, or \c NULL if memory allocation
 *           failed.
 */
Sample* new_Sample(void);


/**
 * Gets the file format of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The Sample format.
 */
Sample_format Sample_get_format(Sample* sample);


/**
 * Loads contents from a file into a Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param in       The input file -- must not be \c NULL.
 * \param format   The input file format -- must be valid.
 */
bool Sample_load(Sample* sample, FILE* in, Sample_format format);


/**
 * Loads a given path into a Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param path     The input file path -- must not be \c NULL.
 * \param format   The input file format -- must be valid.
 */
bool Sample_load_path(Sample* sample, char* path, Sample_format format);


/**
 * Gets the path of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The path.
 */
char* Sample_get_path(Sample* sample);


/**
 * Sets the middle frequency of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param freq     The middle frequency -- must be > \c 0.
 */
void Sample_set_freq(Sample* sample, double freq);


/**
 * Gets the middle frequency of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The middle frequency.
 */
double Sample_get_freq(Sample* sample);


/**
 * Gets the length of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The length in frames/buffer.
 */
uint64_t Sample_get_len(Sample* sample);


/**
 * Sets the loop mode of the Sample.
 *
 * If the loop becomes enabled, the end points may be changed.
 * The loop will not be enabled if the length of the sample is 0.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param loop     The loop mode -- must be \c SAMPLE_LOOP_OFF,
 *                 \c SAMPLE_LOOP_UNI or \c SAMPLE_LOOP_BI.
 */
void Sample_set_loop(Sample* sample, Sample_loop loop);


/**
 * Gets the loop mode of the Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The loop mode.
 */
Sample_loop Sample_get_loop(Sample* sample);


/**
 * Sets the start point of the Sample loop.
 *
 * If the start point is set past the end of the loop or Sample, the loop is
 * turned off.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param start    The loop start -- must not be \c NULL.
 */
void Sample_set_loop_start(Sample* sample, uint64_t start);


/**
 * Gets the start point of the Sample loop.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The loop start.
 */
uint64_t Sample_get_loop_start(Sample* sample);


/**
 * Sets the end point of the Sample loop.
 *
 * If the end point is set at or before the start point or past the length of
 * the Sample, the loop is turned off.
 *
 * \param sample   The Sample -- must not be \c NULL.
 * \param end      The loop end -- must not be \c NULL.
 */
void Sample_set_loop_end(Sample* sample, uint64_t end);


/**
 * Gets the end point of the Sample loop.
 *
 * \param sample   The Sample -- must not be \c NULL.
 *
 * \return   The loop start.
 */
uint64_t Sample_get_loop_end(Sample* sample);


/**
 * Mixes a Sample.
 *
 * \param sample        The Sample -- must not be \c NULL.
 * \param gen           The Generator containing the Sample -- must not be
 *                      \c NULL.
 * \param state         The Voice state -- must not be \c NULL.
 * \param nframes       The number of frames to be mixed.
 * \param offset        The buffer offset.
 * \param freq          The mixing frequency -- must be > \c 0.
 * \param tempo         The tempo -- must be > \c 0.
 * \param middle_tone   The frequency of the sound in the native speed of the
 *                      Sample -- must be > \c 0.
 * \param middle_freq   The mixing speed of the Sample used for playing
 *                      \a middle_tone -- must be > \c 0.
 */
uint32_t Sample_mix(Sample* sample,
                    Generator* gen,
                    Voice_state* state,
                    uint32_t nframes,
                    uint32_t offset,
                    uint32_t freq,
                    double tempo,
                    int buf_count,
                    kqt_frame** bufs,
                    double middle_tone,
                    double middle_freq);


/**
 * Destroys a Sample.
 *
 * \param sample   The Sample -- must not be \c NULL.
 */
void del_Sample(Sample* sample);


#endif // K_SAMPLE_H


