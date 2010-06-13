

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
#define PCM_EXPRESSIONS_MAX (16)
#define PCM_RANDOMS_MAX (8)


typedef struct Generator_pcm
{
    Generator parent;
} Generator_pcm;


/**
 * Creates a new PCM Generator.
 *
 * \param ins_params   The Instrument parameters -- must not be \c NULL.
 * \param gen_params   The Generator parameters -- must not be \c NULL.
 *
 * \return   The new PCM Generator if successful, or \c NULL if memory
 *           allocation failed.
 */
Generator* new_Generator_pcm(Instrument_params* ins_params,
                             Generator_params* gen_params);


uint32_t Generator_pcm_mix(Generator* gen,
                           Voice_state* state,
                           uint32_t nframes,
                           uint32_t offset,
                           uint32_t freq,
                           double tempo,
                           int buf_count,
                           kqt_frame** bufs);


/**
 * Sets a Sample mapping.
 *
 * \param gen            The Generator -- must not be \c NULL and must be a
 *                       PCM Generator.
 * \param source         The (virtual) sound source of the Generator -- must
 *                       be < \c PCM_SOURCES_MAX. This is 0 in most cases
 *                       but may be used to distinguish between e.g. different
 *                       strings in a stringed instrument.
 * \param expr           The (virtual) expression of the Generator -- must be
 *                       < \c PCM_EXPRESSIONS_MAX. This is 0 in most cases but
 *                       may be used to distinguish between e.g. different
 *                       playing techniques.
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
                                        uint8_t source,
                                        uint8_t expr,
                                        double force,
                                        double freq,
                                        uint8_t index,
                                        uint16_t sample,
                                        double sample_freq,
                                        double vol_scale);


/**
 * Removes a Sample mapping.
 *
 * \param gen      The Generator -- must not be \c NULL and must be a
 *                 PCM Generator.
 * \param source   The (virtual) sound source of the Generator -- must
 *                 be < \c PCM_SOURCES_MAX. This is 0 in most cases
 *                 but may be used to distinguish between e.g. different
 *                 strings in a stringed instrument.
 * \param expr     The (virtual) expression of the Generator -- must be
 *                 < \c PCM_EXPRESSIONS_MAX. This is 0 in most cases but
 *                 may be used to distinguish between e.g. different
 *                 playing techniques.
 * \param force    The middle force setting -- must be finite.
 * \param freq     The middle frequency -- must be > \c 0.
 * \param index    The index of the entry -- must be < \c PCM_RANDOMS_MAX.
 *
 * \return   \c true if the mapping changed, otherwise \c false.
 */
bool Generator_pcm_del_sample_mapping(Generator_pcm* pcm,
                                      uint8_t source,
                                      uint8_t expr,
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


