

/*
 * Copyright 2008 Tomi Jylhä-Ollila
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


#ifndef K_INSTRUMENT_H
#define K_INSTRUMENT_H


#include <wchar.h>

#include <frame_t.h>
#include <Event_queue.h>
#include <Voice_state.h>
#include <Note_table.h>
#include <Song_limits.h>


typedef enum
{
	/// Not a valid type.
	INS_TYPE_NONE = 0,
	/// A type used for debugging.
	/// Output is a narrow pulse wave (with one sample value 1, the rest are
	/// 0.5) that lasts no more than 10 phase cycles. Note Off lasts no more
	/// than two phase cycles with all sample values negated.
	INS_TYPE_DEBUG,
	/// A simple sine wave instrument for testing by ear.
	INS_TYPE_SINE,
	/// A sample-based type common in tracker programs.
	INS_TYPE_PCM,
	/// A type for reading audio data from disk -- used for large audio files.
	INS_TYPE_PCM_DISK,
	/// An implementation of Paul Nasca's PADsynth algorithm.
	INS_TYPE_PADSYNTH,
	/// Sentinel -- never used as a valid type.
	INS_TYPE_LAST
} Ins_type;


typedef struct Instrument_field
{
	char* category;
	char* param_name;
	char* param_type;
	struct Instrument_field* field;
} Instrument_field;


typedef struct Instrument
{
	/// Instrument type.
	Ins_type type;
	/// The name of the Instrument.
	wchar_t name[INS_NAME_MAX];
	/// Mixing buffer used (same as either \a pbuf or \a gbuf).
	frame_t** bufs;
	/// Private mixing buffer (required when Instrument-level effects are used).
	frame_t** pbufs;
	/// Global mixing buffer.
	frame_t** gbufs;
	/// Mixing buffer length.
	uint32_t buf_len;
	/// Instrument event queue (esp. pedal events go here).
	Event_queue* events;
	/// An indirect reference to the current Note table used.
	Note_table** notes;
	/// Type-specific data.
	void* type_data;
	/// Description of type-specific settings.
	Instrument_field* type_desc;
	/// Function for getting a type-specific field.
	bool (*get_field)(struct Instrument*, int index, void* data, char** type);
	/// Function for setting a type-specific field.
	bool (*set_field)(struct Instrument*, int index, void* data);
	/// Initialiser for type-specific data -- returns \c 0 on success.
	int (*init)(struct Instrument*);
	/// Uninitialiser for type-specific data.
	void (*uninit)(struct Instrument*);
	/// Mixing algorithm used.
	void (*mix)(struct Instrument*, Voice_state*, uint32_t, uint32_t, uint32_t);
} Instrument;


/**
 * Creates a new Instrument.
 *
 * \param type      The type of the Instrument -- must be a valid type.
 * \param bufs      The global mixing buffers -- must not be \c NULL.
 *                  Additionally, bufs[0] and bufs[1] must not be \c NULL.
 * \param buf_len   The length of a mixing buffer -- must be > \c 0.
 * \param events    The maximum number of events per tick -- must be > \c 0.
 *
 * \return   The new Instrument if successful, or \c NULL if memory allocation
 *           failed.
 */
Instrument* new_Instrument(Ins_type type,
		frame_t** bufs,
		uint32_t buf_len,
		uint8_t events);


/**
 * Gets the type of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The Instrument type.
 */
Ins_type Instrument_get_type(Instrument* ins);


/**
 * Gets the type description of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The type description. If the Instrument doesn't have
 *           type-specific fields, \c NULL is returned.
 */
Instrument_field* Instrument_get_type_desc(Instrument* ins);


/**
 * Gets a type-specific field value of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the field -- must be >= \c 0.
 * \param data    The storage destination for the value.
 * \param type    The storage destination for the type of \a data.
 *
 * \return   \c true if a value was correctly returned. Otherwise \c false is
 *           returned and \a data remains unchanged.
 */
bool Instrument_get_field(Instrument* ins, int index, void* data, char** type);


/**
 * Sets a type-specific field value of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param index   The index of the field -- must be >= \c 0.
 * \param data    The new value.
 *
 * \return   \c true if \a data was correctly set, otherwise \c false.
 */
bool Instrument_set_field(Instrument* ins, int index, void* data);


/**
 * Sets the name of the Instrument.
 *
 * \param ins    The Instrument -- must not be \c NULL.
 * \param name   The name -- must not be \c NULL.
 */
void Instrument_set_name(Instrument* ins, wchar_t* name);


/**
 * Gets the name of the Instrument.
 *
 * \param ins   The Instrument -- must not be \c NULL.
 *
 * \return   The name.
 */
wchar_t* Instrument_get_name(Instrument* ins);


/**
 * Sets the active Note table of the Instrument.
 *
 * \param ins     The Instrument -- must not be \c NULL.
 * \param notes   The indirect reference to the Note table -- must not be
 *                \c NULL.
 */
void Instrument_set_note_table(Instrument* ins, Note_table** notes);


/**
 * Handles a given note as appropriate for the Instrument.
 *
 * \param ins      The Instrument -- must not be \c NULL.
 * \param state    The Voice state -- must not be \c NULL.
 * \param note     The note number -- must be >= \c 0 and
 *                 < \c NOTE_TABLE_NOTES.
 * \param mod      The note modifier -- must be < \c NOTE_TABLE_NOTE_MODS.
 *                 Negative value means that no modifier will be applied.
 * \param octave   The octave -- must be >= \c NOTE_TABLE_OCTAVE_FIRST
 *                 and <= \c NOTE_TABLE_OCTAVE_LAST.
 */
void Instrument_process_note(Instrument* ins,
		Voice_state* state,
		int note,
		int mod,
		int octave);


/**
 * Mixes the Instrument.
 *
 * \param ins       The Instrument -- must not be \c NULL.
 * \param state     The Voice state -- must not be \c NULL.
 * \param nframes   The number of frames to be mixed -- must not be greater
 *                  than the mixing buffer size.
 * \param offset    The starting frame offset (\a nframes - \a offset are
 *                  actually mixed).
 * \param freq      The mixing frequency -- must be > \c 0.
 */
void Instrument_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq);


/**
 * Destroys an existing Instrument.
 *
 * \param   The Instrument -- must not be \c NULL.
 */
void del_Instrument(Instrument* ins);


#endif // K_INSTRUMENT_H


