

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


#ifndef K_PROCESSOR_H
#define K_PROCESSOR_H


#include <debug/assert.h>
#include <Decl.h>
#include <devices/Au_params.h>
#include <devices/Device.h>
#include <devices/Device_params.h>
#include <kunquat/limits.h>
#include <player/devices/Au_state.h>
#include <player/devices/Proc_state.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffers.h>

#include <stdbool.h>
#include <stdint.h>


typedef enum
{
    VOICE_FEATURE_PITCH,
    VOICE_FEATURE_FORCE,
    VOICE_FEATURE_CUT,
    VOICE_FEATURE_FILTER,
    VOICE_FEATURE_PANNING,
    VOICE_FEATURE_COUNT_
} Voice_feature;

static_assert(
        VOICE_FEATURE_COUNT_ <= 31,
        "Too many voice features defined, change flag container type");

#define VOICE_FEATURE_FLAG(feature) (1 << (feature))

#define VOICE_FEATURES_ALL ((1 << VOICE_FEATURE_COUNT_) - 1)


/**
 * Processor creates signal output based on voice or signal input.
 */
struct Processor
{
    Device parent;
    int index;
    const Au_params* au_params;
    uint32_t voice_features[KQT_DEVICE_PORTS_MAX];

    bool enable_voice_support;
    bool enable_signal_support;

    Voice_state_init_func* init_vstate;
};


/**
 * Create a new Processor of the specified type.
 *
 * \param index       The Processor index in the containing Processor table
 *                    -- must be >= \c 0 and < \c KQT_PROCESSORS_MAX.
 * \param au_params   The Audio unit parameters -- must not be \c NULL.
 *
 * \return   The new Processor if successful, or \c NULL if memory allocation
 *           failed.
 */
Processor* new_Processor(int index, const Au_params* au_params);


/**
 * Initialise the general Processor parameters.
 *
 * \param proc          The Processor -- must not be \c NULL.
 * \param destroy       The destructor of the Processor -- must not be \c NULL.
 * \param init_vstate   The Voice state initialiser, or \c NULL if not needed.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Processor_init(
        Processor* proc,
        //void (*destroy)(Processor*),
        Voice_state_init_func* init_vstate);


/**
 * Set a voice feature of the Processor.
 *
 * \param proc       The Processor -- must not be \c NULL.
 * \param port_num   The output port number -- must be >= \c 0 and
 *                   < \c KQT_DEVICE_PORTS_MAX.
 * \param feature    The voice feature -- must be valid.
 * \param enabled    Whether \a feature is enabled or not.
 */
void Processor_set_voice_feature(
        Processor* proc, int port_num, Voice_feature feature, bool enabled);


/**
 * Get a voice feature enabled status of the Processor.
 *
 * \param proc       The Processor -- must not be \c NULL.
 * \param port_num   The output port number -- must be >= \c 0 and
 *                   < \c KQT_DEVICE_PORTS_MAX.
 * \param feature    The voice feature -- must be valid.
 *
 * \return   \c true if \a feature is enabled, otherwise \c false.
 */
bool Processor_is_voice_feature_enabled(
        const Processor* proc, int port_num, Voice_feature feature);


/**
 * Set Voice signal support.
 *
 * Note that Voice signals may be always disabled for certain Processor types.
 *
 * \param proc      The Processor -- must not be \c NULL.
 * \param enabled   \c true if Voice signals should be enabled, otherwise
 *                  \c false.
 */
void Processor_set_voice_signals(Processor* proc, bool enabled);


/**
 * Get Voice signal support information.
 *
 * \param proc   The Processor -- must not be \c NULL.
 *
 * \return   \c true if \a proc has Voice signals enabled, otherwise \c false.
 */
bool Processor_get_voice_signals(const Processor* proc);


/**
 * Get the Audio unit parameters associated with the Processor.
 *
 * \param proc   The Processor -- must not be \c NULL.
 *
 * \return   The Audio unit parameters.
 */
const Au_params* Processor_get_au_params(const Processor* proc);


/**
 * Process a Voice state that belongs to the Processor.
 *
 * \param proc         The Processor -- must not be \c NULL.
 * \param dstates      The Device states -- must not be \c NULL.
 * \param vstate       The Voice state -- must not be \c NULL.
 * \param wbs          The Work buffers -- must not be \c NULL.
 * \param buf_start    The start index of the buffer area to be processed.
 * \param buf_stop     The stop index of the buffer area to be processed.
 * \param audio_rate   The mixing frequency -- must be > \c 0.
 * \param tempo        The current tempo -- must be > \c 0.
 */
void Processor_process_vstate(
        const Processor* proc,
        Device_states* dstates,
        Voice_state* vstate,
        const Work_buffers* wbs,
        int32_t buf_start,
        int32_t buf_stop,
        uint32_t audio_rate,
        double tempo);


/**
 * Destroy an existing Processor.
 *
 * \param proc   The Processor, or \c NULL.
 */
void del_Processor(Processor* proc);


#endif // K_PROCESSOR_H


