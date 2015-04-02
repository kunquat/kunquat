

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


#ifndef K_VOICE_H
#define K_VOICE_H


#include <stdbool.h>
#include <stdint.h>

#include <devices/Processor.h>
#include <mathnum/Random.h>
#include <player/Channel_proc_state.h>
#include <player/Device_states.h>
#include <player/Voice_state.h>
#include <player/Work_buffers.h>


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
    uint64_t id;             ///< An identification number for this initialisation.
    uint64_t group_id;       ///< The ID of the group this Voice currently belogns to.
    Voice_prio prio;         ///< Current priority of the Voice.
    const Processor* proc;   ///< The Processor.
    size_t state_size;       ///< The amount bytes allocated for the Voice state.
    Voice_state* state;      ///< The current playback state.
    Random* rand_p;          ///< Parameter random source.
    Random* rand_s;          ///< Signal random source.
} Voice;


/**
 * Create a new Voice.
 *
 * \param state_size   The amount of bytes to reserve for Voice states.
 *
 * \return   The new Voice if successful, or \c NULL if memory allocation
 *           failed.
 */
Voice* new_Voice(void);


/**
 * Reserve space for the Voice state.
 *
 * \param voice        The Voice -- must not be \c NULL.
 * \param state_size   The amount of bytes to reserve for the Voice state.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Voice_reserve_state_space(Voice* voice, size_t state_size);


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
 * Retrieve the Voice identification.
 *
 * The user should store the ID after retrieving the Voice from the Voice
 * pool.
 *
 * \param voice   The Voice -- must not be \c NULL.
 *
 * \return   The ID.
 */
uint64_t Voice_id(const Voice* voice);


/**
 * Get the group ID of the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 *
 * \return   The group ID.
 */
uint64_t Voice_get_group_id(const Voice* voice);


/**
 * Get the Processor associated with the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 *
 * \return   The Processor.
 */
const Processor* Voice_get_proc(const Voice* voice);


/**
 * Initialise the Voice for mixing.
 *
 * \param voice        The Voice -- must not be \c NULL.
 * \param proc         The Processor used -- must not be \c NULL.
 * \param group_id     The ID of the group this Voice belongs to. This is used
 *                     to identify which Voices are connected.
 * \param proc_state   The Processor state -- must not be \c NULL.
 * \param cpstate      The Channel-specific Processor state -- must not be
 *                     \c NULL.
 * \param seed         The random seed.
 * \param freq         The mixing frequency -- must be > \c 0.
 * \param tempo        The current tempo -- must be > \c 0.
 */
void Voice_init(
        Voice* voice,
        const Processor* proc,
        uint64_t group_id,
        const Proc_state* proc_state,
        Channel_proc_state* cpstate,
        uint64_t seed,
        uint32_t freq,
        double tempo);


/**
 * Reset the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL.
 */
void Voice_reset(Voice* voice);


/**
 * Mix the Voice.
 *
 * \param voice    The Voice -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 * \param wbs      The Work buffers -- must not be \c NULL.
 * \param amount   The number of frames to be mixed.
 * \param offset   The buffer offset.
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param tempo    The current tempo -- must be > \c 0.
 */
void Voice_mix(
        Voice* voice,
        Device_states* states,
        const Work_buffers* wbs,
        uint32_t amount,
        uint32_t offset,
        uint32_t freq,
        double tempo);


/**
 * Return the actual current force of the Voice.
 *
 * \param voice   The Voice -- must not be \c NULL and must be active.
 *
 * \return   The actual force.
 */
double Voice_get_actual_force(const Voice* voice);


/**
 * Destroy an existing Voice.
 *
 * \param voice   The Voice, or \c NULL.
 */
void del_Voice(Voice* voice);


#endif // K_VOICE_H


