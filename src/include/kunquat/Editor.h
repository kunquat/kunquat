

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


#ifndef KQT_EDITOR_H
#define KQT_EDITOR_H


#ifdef __cplusplus
extern "C" {
#endif


#include <kunquat/Handle.h>


typedef enum
{
    EVENT_NONE = 0,                         ///< An uninitialised event.
    EVENT_GENERAL_COND,                     ///< Evaluate a conditional expression.
    EVENT_GENERAL_LAST               =  63, ///< Sentinel -- never used as a valid type.
                                     
    EVENT_GLOBAL_SET_TEMPO           =  64, ///< Set tempo. (BPM (float))
    EVENT_GLOBAL_SLIDE_TEMPO         =  65, ///< Slide tempo.
    EVENT_GLOBAL_SLIDE_TEMPO_LENGTH  =  66,
    EVENT_GLOBAL_PATTERN_DELAY       =  67, ///< Pattern delay.
                                     
    EVENT_GLOBAL_SET_VOLUME          =  72, ///< Set global volume.
    EVENT_GLOBAL_SLIDE_VOLUME        =  73, ///< Slide global volume.
    EVENT_GLOBAL_SLIDE_VOLUME_LENGTH =  74,
                                     
    EVENT_GLOBAL_SET_SCALE           =  80, ///< Set default scale used by Instruments.
    EVENT_GLOBAL_RETUNE_SCALE        =  81, ///< Retune scale.

    EVENT_GLOBAL_SET_JUMP_SUBSONG    =  88,
    EVENT_GLOBAL_SET_JUMP_SECTION    =  89,
    EVENT_GLOBAL_SET_JUMP_ROW        =  90,
    EVENT_GLOBAL_SET_JUMP_COUNTER    =  91,
    EVENT_GLOBAL_JUMP                =  92,
                                     
    EVENT_GLOBAL_SET_VAR,                   ///< Set a variable.
                                     
    EVENT_GLOBAL_LAST                = 127, ///< Sentinel -- never used as a valid type.
                                     
    EVENT_VOICE_NOTE_ON              = 128, ///< Note On event. (note, modifier, octave)
    EVENT_VOICE_NOTE_OFF             = 129, ///< Note Off event.
                                     
    EVENT_VOICE_SET_FORCE            = 136, ///< Set Force.
    EVENT_VOICE_SLIDE_FORCE          = 137, ///< Slide Force.
    EVENT_VOICE_SLIDE_FORCE_LENGTH   = 138,
    EVENT_VOICE_TREMOLO_SPEED        = 139, ///< Tremolo speed.
    EVENT_VOICE_TREMOLO_DEPTH        = 140, ///< Tremolo depth.
    EVENT_VOICE_TREMOLO_DELAY        = 141, ///< Tremolo delay.
                                     
    EVENT_VOICE_SLIDE_PITCH          = 144, ///< Slide pitch.
    EVENT_VOICE_SLIDE_PITCH_LENGTH   = 145,
    EVENT_VOICE_VIBRATO_SPEED        = 146, ///< Vibrato speed.
    EVENT_VOICE_VIBRATO_DEPTH        = 147, ///< Vibrato depth.
    EVENT_VOICE_VIBRATO_DELAY        = 148, ///< Vibrato delay.
    EVENT_VOICE_ARPEGGIO             = 149, ///< Arpeggio (the retro effect).
                                     
    EVENT_VOICE_SET_FILTER           = 152, ///< Set filter cut-off.
    EVENT_VOICE_SLIDE_FILTER         = 153, ///< Slide filter cut-off.
    EVENT_VOICE_SLIDE_FILTER_LENGTH  = 154,
    EVENT_VOICE_AUTOWAH_SPEED        = 155, ///< Auto-wah (filter cut-off oscillation) speed.
    EVENT_VOICE_AUTOWAH_DEPTH        = 156, ///< Auto-wah depth.
    EVENT_VOICE_AUTOWAH_DELAY        = 157, ///< Auto-wah delay.
    EVENT_VOICE_SET_RESONANCE        = 158, ///< Set filter resonance (Q factor).
                                     
    EVENT_VOICE_SET_PANNING          = 160, ///< Set panning position.
    EVENT_VOICE_SLIDE_PANNING        = 161, ///< Slide panning position.
    EVENT_VOICE_SLIDE_PANNING_LENGTH = 162,
                                    
    EVENT_VOICE_LAST                 = 255, ///< Sentinel -- never used as a valid type.
                                    
    EVENT_INS_SET_PEDAL              = 256, ///< Set Instrument pedal.
                                    
    EVENT_INS_LAST                   = 287, ///< Sentinel -- never used as a valid type.
                                    
    EVENT_CHANNEL_SET_INSTRUMENT     = 288, ///< Set Instrument.
                                    
    EVENT_LAST                           ///< Sentinel -- never used as a valid type.
} Event_type;


/**
 * \defgroup Editor Editing Kunquat compositions
 *
 * \{
 *
 * \brief
 * This module describes the API for applications that modify Kunquat
 * compositions.
 */


/**
 * Initialises a read/write/commit Kunquat Handle from a composition state.
 *
 * The current implementation limits the maximum number of simultaneous
 * Kunquat Handles to \c KQT_HANDLES_MAX.
 *
 * \param buffer_size   The size of the mixing buffers -- should be positive.
 *                      See kqt_new_Handle for detailed explanation.
 * \param path          The path to the Kunquat composition state directory --
 *                      should not be \c NULL.
 *
 * \return   The new read/write/commit Kunquat Handle if successful, otherwise
 *           \c NULL  (check kqt_Handle_get_error(\c NULL) for error message).
 */
kqt_Handle* kqt_new_Handle_rwc(long buffer_size, char* path);


/**
 * Commits changes made to the Kunquat Handle.
 *
 * \param handle   The Handle -- should not be \c NULL.
 *
 * \return   \c 1 if successful, or \c 0 if failed.
 */
int kqt_Handle_commit(kqt_Handle* handle);


/**
 * Inserts an Event inside the Kunquat Handle.
 *
 * This function only synchronises the player state! The user must
 * additionally call kqt_Handle_write_column to actually make the
 * changes to the data in the composition state.
 *
 * \param handle      The Handle -- should not be \c NULL.
 * \param pattern     The number of the pattern -- should be >= \c 0 and
 *                    < \c KQT_PATTERNS_MAX.
 * \param column      The number of the column -- should be >= \c -1 and
 *                    < \c KQT_COLUMNS_MAX. The index \c -1 is the global
 *                    column.
 * \param beat        The index of the beat -- should be >= \c 0.
 * \param remainder   The remainder of the beat -- should be >= \c 0 and
 *                    < \c KQT_RELTIME_BEAT.
 * \param index       The 0-based index of the Event in the row.
 * \param type        The type of the Event.
 * \param fields      The fields of the Event -- should correspond to the
 *                    type description of the Event.
 *
 * \return   \c 1 if successful, or \c 0 if failed.
 */
int kqt_Handle_insert_event(kqt_Handle* handle,
                            int pattern,
                            int column,
                            long long beat,
                            long remainder,
                            int index,
                            Event_type type,
                            char* fields);


/**
 * Writes a column into the composition state.
 *
 * \param handle      The Handle -- should not be \c NULL.
 * \param pattern     The number of the pattern -- should be >= \c 0 and
 *                    < \c KQT_PATTERNS_MAX.
 * \param column      The number of the column -- should be >= \c -1 and
 *                    < \c KQT_COLUMNS_MAX. The index \c -1 is the global
 *                    column.
 *
 * \return   \c 1 if successful, or \c 0 if failed.
 */
int kqt_Handle_write_column(kqt_Handle* handle,
                            int pattern,
                            int column);


/* \} */


#ifdef __cplusplus
}
#endif


#endif // KQT_EDITOR_H


