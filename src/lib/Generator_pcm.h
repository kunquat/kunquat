

/*
 * Copyright 2009 Tomi Jylhä-Ollila
 *
 * This file is part of Kunquat.
 *
 * Kunquat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kunquat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Kunquat.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef K_GENERATOR_PCM_H
#define K_GENERATOR_PCM_H


#include <stdint.h>
#include <math.h>

#include <Sample.h>
#include <Generator.h>
#include <Voice_state.h>
#include <AAtree.h>


#define PCM_SAMPLES_MAX (512)

#define PCM_SOURCES_MAX (16)
#define PCM_STYLES_MAX (16)
#define PCM_RANDOMS_MAX (8)


typedef struct Sample_entry
{
    double freq;
    double vol_scale;
    uint16_t sample;
} Sample_entry;


typedef struct Random_list
{
    pitch_t freq;
    double freq_tone;
    double force;
    int entry_count;
    Sample_entry entries[PCM_RANDOMS_MAX];
} Random_list;


typedef struct Generator_pcm
{
    Generator parent;
    AAiter* iter;
    AAtree* maps[PCM_SOURCES_MAX * PCM_STYLES_MAX];
    Sample* samples[PCM_SAMPLES_MAX];
} Generator_pcm;


/**
 * Creates a new PCM Generator.
 *
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 *
 * \return   The new PCM Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator_pcm* new_Generator_pcm(Instrument_params* ins_params);


uint32_t Generator_pcm_mix(Generator* gen,
                           Voice_state* state,
                           uint32_t nframes,
                           uint32_t offset,
                           uint32_t freq,
                           int buf_count,
                           frame_t** bufs);


/**
 * Sets a Sample in the PCM Generator.
 *
 * \param gen      The Generator -- must not be \c NULL and must be a PCM
 *                 Generator.
 * \param index    The destination index -- must be >= \c 0 and
 *                 < \c PCM_SAMPLES_MAX. Any previously loaded Sample in the
 *                 index will be removed.
 * \param sample   The Sample -- must not be \c NULL.
 */
void Generator_pcm_set_sample(Generator_pcm* pcm,
                              uint16_t index,
                              Sample* sample);


/**
 * Gets a Sample from the PCM Generator.
 *
 * \param gen     The Generator -- must not be \c NULL and must be a PCM
 *                Generator.
 * \param index   The destination index -- must be >= \c 0 and
 *                < \c PCM_SAMPLES_MAX.
 *
 * \return   The Sample if one exists, otherwise \c NULL.
 */
Sample* Generator_pcm_get_sample(Generator_pcm* pcm, uint16_t index);


/**
 * Sets the middle frequency of a Sample.
 *
 * \param gen     The Generator -- must not be \c NULL and must be a PCM
 *                Generator.
 * \param index   The destination index -- must be >= \c 0 and
 *                < \c PCM_SAMPLES_MAX.
 * \param freq    The middle frequency -- must be > \c 0.
 */
void Generator_pcm_set_sample_freq(Generator_pcm* pcm,
                                   uint16_t index,
                                   double freq);


/**
 * Gets the middle frequency of a Sample.
 *
 * \param gen     The Generator -- must not be \c NULL and must be a PCM
 *                Generator.
 * \param index   The destination index -- must be >= \c 0 and
 *                < \c PCM_SAMPLES_MAX.
 *
 * \return   The middle frequency of the sample if one exists, otherwise \c 0.
 */
double Generator_pcm_get_sample_freq(Generator_pcm* pcm, uint16_t index);


/**
 * Sets a Sample mapping.
 *
 * \param gen            The Generator -- must not be \c NULL and must be a
 *                       PCM Generator.
 * \param source         The (virtual) sound source of the Instrument -- must
 *                       be < \c PCM_SOURCES_MAX. This is 0 in most cases
 *                       but may be used to distinguish between e.g. different
 *                       strings in a stringed instrument.
 * \param style          The (virtual) style of the Instrument -- must be
 *                       < \c PCM_STYLES_MAX. This is 0 in most cases but
 *                       may be used to distinguish between e.g. different
 *                       playing styles.
 * \param force          The middle force -- must be finite.
 * \param freq           The middle frequency -- must be > \c 0.
 * \param index          The index of the entry -- must be
 *                       < \c PCM_RANDOMS_MAX. If there are Samples defined
 *                       for multiple indices, one is chosen randomly.
 * \param sample         The index of the actual Sample in the Sample table --
 *                       must be < \c PCM_SAMPLES_MAX.
 * \param sample_freq    The Sample frequency in the middle point -- must be
 *                       > \c 0.
 * \param vol_scale      The scale factor used for calculating the Sample
 *                       volume -- must be > \c 0.
 *
 * \return   The actual index entry (see \a index) that was set, or a negative
 *           value if memory allocation failed.
 */
int8_t Generator_pcm_set_sample_mapping(Generator_pcm* pcm,
        uint8_t source, uint8_t style, double force, double freq, uint8_t index,
        uint16_t sample, double sample_freq, double vol_scale);


/**
 * Removes a Sample mapping.
 *
 * \param gen      The Generator -- must not be \c NULL and must be a
 *                 PCM Generator.
 * \param source   The (virtual) sound source of the Instrument -- must
 *                 be < \c PCM_SOURCES_MAX. This is 0 in most cases
 *                 but may be used to distinguish between e.g. different
 *                 strings in a stringed instrument.
 * \param style    The (virtual) style of the Instrument -- must be
 *                 < \c PCM_STYLES_MAX. This is 0 in most cases but
 *                 may be used to distinguish between e.g. different
 *                 playing styles.
 * \param force    The middle force setting -- must be finite.
 * \param freq     The middle frequency -- must be > \c 0.
 * \param index    The index of the entry -- must be < \c PCM_RANDOMS_MAX.
 *
 * \return   \c true if the mapping changed, otherwise \c false.
 */
bool Generator_pcm_del_sample_mapping(Generator_pcm* pcm,
                                      uint8_t source,
                                      uint8_t style,
                                      double force,
                                      double freq,
                                      uint8_t index);


/**
 * Destroys an existing PCM Generator.
 *
 * \param gen   The PCM Generator -- must not be \c NULL.
 */
void del_Generator_pcm(Generator* gen);


#endif // K_GENERATOR_PCM_H


