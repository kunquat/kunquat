

#include <stdlib.h>
#include <assert.h>

#include "Instrument.h"
#include "Instrument_debug.h"

#include <xmemory.h>


Instrument* new_Instrument(Ins_type type, frame_t** bufs, uint32_t buf_len)
{
	assert(type > INS_TYPE_NONE);
	assert(type < INS_TYPE_LAST);
	assert(bufs != NULL);
	assert(bufs[0] != NULL);
	assert(bufs[1] != NULL);
	assert(buf_len > 0);
	Instrument* ins = xalloc(Instrument);
	if (ins == NULL)
	{
		return NULL;
	}
	ins->events = new_Event_queue(16); /// FIXME: customisable?
	if (ins->events == NULL)
	{
		xfree(ins);
		return NULL;
	}
	ins->type = type;
	ins->pbufs = NULL;
	ins->bufs = ins->gbufs = bufs;
	ins->buf_len = buf_len;
	switch (type)
	{
		case INS_TYPE_DEBUG:
			ins->mix = Instrument_debug_mix;
			break;
		default:
			ins->mix = NULL;
	}
	return ins;
}


void Instrument_mix(Instrument* ins,
		Voice_state* state,
		uint32_t nframes,
		uint32_t offset,
		uint32_t freq)
{
	assert(ins != NULL);
	assert(state != NULL);
	assert(nframes <= ins->buf_len);
	assert(freq > 0);
	assert(ins->mix != NULL);
	if (!state->active)
	{
		return;
	}
	ins->mix(ins, state, nframes, offset, freq);
	return;
}


void del_Instrument(Instrument* ins)
{
	assert(ins != NULL);
	xfree(ins->events);
	if (ins->pbufs != NULL)
	{
		assert(ins->pbufs[0] != NULL);
		assert(ins->pbufs[1] != NULL);
		xfree(ins->pbufs[0]);
		xfree(ins->pbufs[1]);
		xfree(ins->pbufs);
	}
	xfree(ins);
}


