

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


#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "Listener.h"
#include "Listener_ins.h"
#include "Listener_ins_pcm.h"

#include <Song.h>
#include <Ins_table.h>
#include <Instrument.h>
#include <Instrument_pcm.h>
#include <Song_limits.h>


int Listener_ins_pcm_load_sample(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argc;
	(void)msg;
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t song_id = argv[0]->i;
	int32_t ins_num = argv[1]->i;
	Instrument* ins = NULL;
	if (!ins_get(lr, song_id, ins_num, &ins))
	{
		return 0;
	}
	check_cond(lr, ins != NULL,
			"The Instrument #%ld", (long)ins_num);
	check_cond(lr, Instrument_get_type(ins) == INS_TYPE_PCM,
			"The Instrument type (%d)", (int)Instrument_get_type(ins));
	int32_t sample_index = argv[2]->i;
	check_cond(lr, sample_index >= 0 && sample_index < PCM_SAMPLES_MAX,
			"The Sample index (%ld)", (long)sample_index);
	char* spath = &argv[3]->s;
	if (!Instrument_pcm_set_sample(ins, sample_index, spath))
	{
		lo_message m = new_msg();
		lo_message_add_string(m, "Couldn't load sample");
		lo_message_add_string(m, spath);
		int ret = 0;
		send_msg(lr, "error", m, ret);
		lo_message_free(m);
	}
	ins_info(lr, song_id, ins_num, ins);
	return 0;
}


int Listener_ins_pcm_sample_set_mid_freq(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argc;
	(void)msg;
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t song_id = argv[0]->i;
	int32_t ins_num = argv[1]->i;
	Instrument* ins = NULL;
	if (!ins_get(lr, song_id, ins_num, &ins))
	{
		return 0;
	}
	check_cond(lr, ins != NULL,
			"The Instrument #%ld", (long)ins_num);
	check_cond(lr, Instrument_get_type(ins) == INS_TYPE_PCM,
			"The Instrument type (%d)", (int)Instrument_get_type(ins));
	int32_t sample_index = argv[2]->i;
	check_cond(lr, sample_index >= 0 && sample_index < PCM_SAMPLES_MAX,
			"The Sample index (%ld)", (long)sample_index);
	double freq = argv[3]->d;
	check_cond(lr, freq > 0, "The frequency (%f)", freq);
	Instrument_pcm_set_sample_freq(ins, sample_index, freq);
	ins_info(lr, song_id, ins_num, ins);
	return 0;
}


int Listener_ins_pcm_remove_sample(const char* path,
		const char* types,
		lo_arg** argv,
		int argc,
		lo_message msg,
		void* user_data)
{
	(void)path;
	(void)types;
	(void)argc;
	(void)msg;
	assert(user_data != NULL);
	Listener* lr = user_data;
	if (lr->host == NULL)
	{
		return 0;
	}
	assert(lr->method_path != NULL);
	int32_t song_id = argv[0]->i;
	int32_t ins_num = argv[1]->i;
	Instrument* ins = NULL;
	if (!ins_get(lr, song_id, ins_num, &ins))
	{
		return 0;
	}
	check_cond(lr, ins != NULL,
			"The Instrument #%ld", (long)ins_num);
	check_cond(lr, Instrument_get_type(ins) == INS_TYPE_PCM,
			"The Instrument type (%d)", (int)Instrument_get_type(ins));
	int32_t sample_index = argv[2]->i;
	check_cond(lr, sample_index >= 0 && sample_index < PCM_SAMPLES_MAX,
			"The Sample index (%ld)", (long)sample_index);
	if (!Instrument_pcm_set_sample(ins, sample_index, NULL))
	{
		lo_message m = new_msg();
		lo_message_add_string(m, "Couldn't remove sample");
		lo_message_add_int32(m, sample_index);
		int ret = 0;
		send_msg(lr, "error", m, ret);
		lo_message_free(m);
	}
	ins_info(lr, song_id, ins_num, ins);
	return 0;
}


