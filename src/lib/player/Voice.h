

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


#ifndef KQT_VOICE_H
#define KQT_VOICE_H


#include <decl.h>
#include <init/devices/Processor.h>
#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <player/Device_states.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


typedef enum
{
    VOICE_PRIO_INACTIVE = 0,
    VOICE_PRIO_BG,
    VOICE_PRIO_FG,
    VOICE_PRIO_NEW
} Voice_prio;


/**
 * This contains all the playback state information of a single note of a
 * Processor being played.
 */
typedef struct Voice
{
    uint64_t group_id;       ///< The ID of the group this Voice currently belongs to.
    int ch_num;              ///< The last Channel that initialised this Voice.
    bool updated;            ///< Used to cut Voices that are not updated.
    Voice_prio prio;         ///< Current priority of the Voice.
    int32_t frame_offset;
    bool use_test_output;
    int test_proc_index;
    const Processor* proc;   ///< The Processor.
    int32_t state_size;      ///< The amount bytes allocated for the Voice state.
    Voice_state* state;      ///< The current playback state.
    Work_buffer* wb;         ///< The Work buffer associated with this Voice.
    Random rand_p;           ///< Parameter random source.
    Random rand_s;           ///< Signal random source.
} Voice;


/**
 * Create a new Voice.
 *
 * \return   The new Voice if successful, or \c NULL if memory allocation
 *           failed.
 */
Voice* new_Voice(void);


/**
 * Reserve space for the Voice state.
 *
 * \param voice        The Voice -- must not be \c NULL.
 * \param state_size   The amount of bytes to reserve for the Voice state
 *                     -- must be >= \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Voice_reserve_state_space(Voice* voice, int32_t state_size);


/**
 * Compare priorities of two Voices.
 *
 * \param v1   The first Voice -- must not be \c NULL.
 * \param v2   The second Voice -- must not be \c NULL.
 *
 * \return   An integer less than, equal to or greater than zero if \a v1 is
 *           found, respectively, to be lower than, equal to or greater than
 *           \a v2 in priority.
 */
int Voice_cmp(const Voice* v1, const Voice* v2);


/**
 * Get the group ID of the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 *
 * \return   The group ID.
 */
uint64_t Voice_get_group_id(const Voice* voice);


/**
 * Get the Channel number associated with the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 *
 * \return   The Channel number, or \c -1 if \a voice is not in use.
 */
int Voice_get_ch_num(const Voice* voice);


/**
 * Get the Processor associated with the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 *
 * \return   The Processor.
 */
const Processor* Voice_get_proc(const Voice* voice);


/**
 * Set the Work buffer associated with the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 * \param wb      The Work buffer, or \c NULL.
 */
void Voice_set_work_buffer(Voice* voice, Work_buffer* wb);


/**
 * Reserve the Voice for a new note.
 *
 * \param voice      The Voice -- must not be \c NULL.
 * \param group_id   The ID of the group this Voice belongs to. This is used
 *                   to identify which Voices are connected.
 * \param ch_num     The Channel number associated with this initialisation
 *                   -- must be >= \c 0 and < \c KQT_CHANNELS_MAX, or \c -1
 *                   which indicates indeterminate Channel.
 */
void Voice_reserve(Voice* voice, uint64_t group_id, int ch_num);


/**
 * Initialise the Voice for mixing.
 *
 * \param voice        The Voice -- must not be \c NULL.
 * \param proc         The Processor used -- must not be \c NULL.
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param seed         The random seed.
 */
void Voice_init(
        Voice* voice,
        const Processor* proc,
        const Proc_state* proc_state,
        uint64_t seed);


/**
 * Set a Processor for test output.
 *
 * \param voice        The Voice -- must not be \c NULL.
 * \param proc_index   The Processor index -- must be >= \c 0 and
 *                     < \c KQT_PROCESSORS_MAX.
 */
void Voice_set_test_processor(Voice* voice, int proc_index);


/**
 * Set a Processor test parameter.
 *
 * \param voice   The Voice -- must not be \c NULL.
 * \param param   The test parameter -- must not be \c NULL and must contain
 *                no more than \c KQT_VAR_NAME_MAX characters.
 */
void Voice_set_test_processor_param(Voice* voice, const char* param);


/**
 * Check Voice test output status.
 *
 * \param voice   The Voice -- must not be \c NULL.
 *
 * \return   \c true if test output Processor is set, otherwise \c false.
 */
bool Voice_is_using_test_output(const Voice* voice);


/**
 * Get test output Processor index.
 *
 * \param voice   The Voice -- must not be \c NULL and must have a test output
 *                Processor index set.
 *
 * \return   The test Processor index.
 */
int Voice_get_test_proc_index(const Voice* voice);


/**
 * Reset the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 */
void Voice_reset(Voice* voice);


/**
 * Render the Voice.
 *
 * \param voice         The Voice, or \c NULL if the associated Processor uses
 *                      stateless Voice rendering.
 * \param proc_id       The Processor ID -- must be valid.
 * \param dstates       The Device states -- must not be \c NULL.
 * \param thread_id     The ID of the thread accessing the Device state
 *                      -- must be a valid ID currently in use.
 * \param wbs           The Work buffers -- must not be \c NULL.
 * \param frame_count   Number of frames to be rendered >= \c 0.
 * \param tempo         The current tempo -- must be > \c 0.
 *
 * \return   The stop index for keeping the Voice alive. This is always
 *           <= \a frame_count]. If equal to \a frame_count, voice processing
 *           may not have finished and therefore should be kept alive.
 */
int32_t Voice_render(
        Voice* voice,
        uint32_t proc_id,
        Device_states* dstates,
        int thread_id,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo);


/**
 * Destroy an existing Voice.
 *
 * \param voice   The Voice, or \c NULL.
 */
void del_Voice(Voice* voice);


#endif // KQT_VOICE_H


