

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "Voice.h"

#include <xmemory.h>


Voice* new_Voice(uint8_t events)
{
	assert(events > 0);
	Voice* voice = xalloc(Voice);
	if (voice == NULL)
	{
		return NULL;
	}
	voice->events = new_Event_queue(events);
	if (voice->events == NULL)
	{
		xfree(voice);
		return NULL;
	}
	voice->id = 0;
	voice->prio = VOICE_PRIO_INACTIVE;
	voice->ins = NULL;
	Voice_state_init(&voice->state);
	return voice;
}


int Voice_cmp(Voice* v1, Voice* v2)
{
	assert(v1 != NULL);
	assert(v2 != NULL);
	return v1->prio - v2->prio;
}


void Voice_set_instrument(Voice* voice, Instrument* ins)
{
	assert(voice != NULL);
	assert(ins != NULL);
	voice->ins = ins;
	return;
}


bool Voice_add_event(Voice* voice, Event* event, uint32_t pos)
{
	assert(voice != NULL);
	assert(event != NULL);
	return Event_queue_ins(voice->events, event, pos);
}


void Voice_mix(Voice* voice,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq)
{
	assert(voice != NULL);
	assert(freq > 0);
	uint32_t mixed = offset;
	Event* next = NULL;
	uint32_t mix_until = nframes;
	bool event_found = Event_queue_get(voice->events, &next, &mix_until);
	while (mixed < nframes || event_found)
	{
		if (mix_until > nframes)
		{
			mix_until = nframes;
		}
/*		fprintf(stderr, "!!! Calling Instrument_mix(%p, %p, %lu, %lu, %lu)\n",
				voice->ins,
				&voice->state,
				(unsigned long)mix_until,
				(unsigned long)mixed,
				(unsigned long)freq);
		fprintf(stderr, "!!! Voice state is: %s, %f, %llu, %lf, %llu, %lf, %s, %llu, %lf\n",
				voice->state.active ? "active" : "inactive",
				voice->state.freq,
				(unsigned long long)voice->state.pos,
				voice->state.pos_part,
				(unsigned long long)voice->state.rel_pos,
				voice->state.rel_pos_part,
				voice->state.note_on ? "Note On" : "Note Off",
				(unsigned long long)voice->state.noff_pos,
				voice->state.noff_pos_part); */
		Instrument_mix(voice->ins, &voice->state, mix_until, mixed, freq);
		if (event_found)
		{
			if (EVENT_TYPE_IS_GENERAL(Event_get_type(next)))
			{
				// TODO: handle general events
			}
			else
			{
				assert(EVENT_TYPE_IS_INS(Event_get_type(next)));
				int64_t note = -1;
				int64_t note_mod = -1;
				int64_t note_octave = -1;
				switch (Event_get_type(next))
				{
					case EVENT_TYPE_NOTE_ON:
						Event_int(next, 0, &note);
						Event_int(next, 1, &note_mod);
						Event_int(next, 2, &note_octave);
						Instrument_process_note(voice->ins,
								&voice->state,
								(int)note,
								(int)note_mod,
								(int)note_octave);
						break;
					case EVENT_TYPE_NOTE_OFF:
						voice->state.note_on = false;
						break;
					default:
						break;
				}
			}
		}
		mixed = mix_until;
		mix_until = nframes;
		event_found = Event_queue_get(voice->events, &next, &mix_until);
	}
	return;
}


void del_Voice(Voice* voice)
{
	assert(voice != NULL);
	del_Event_queue(voice->events);
	xfree(voice);
	return;
}


