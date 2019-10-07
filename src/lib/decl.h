

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef KQT_DECL_H
#define KQT_DECL_H


#define DECLS(name) typedef struct name name

DECLS(Array);
DECLS(Au_event_map);
DECLS(Au_expressions);
DECLS(Au_params);
DECLS(Au_state);
DECLS(Au_streams);
DECLS(Au_table);
DECLS(Audio_unit);
DECLS(Background_loader);
DECLS(Bind);
DECLS(Bit_array);
DECLS(Channel);
DECLS(Channel_event_buffer);
DECLS(Connections);
DECLS(Device);
DECLS(Device_impl);
DECLS(Device_state);
DECLS(Device_states);
DECLS(Device_thread_state);
DECLS(Envelope);
DECLS(Event_names);
DECLS(Event_params);
DECLS(Etable);
DECLS(General_state);
DECLS(Linear_controls);
DECLS(Master_params);
DECLS(Mixed_signal_plan);
DECLS(Module);
DECLS(Param_proc_filter);
DECLS(Proc_state);
DECLS(Processor);
DECLS(Random);
DECLS(Sample);
DECLS(Sample_params);
DECLS(Song);
DECLS(Streader);
DECLS(Tstamp);
DECLS(Tuning_state);
DECLS(Tuning_table);
DECLS(Value);
DECLS(Voice);
DECLS(Voice_group);
DECLS(Voice_signal_plan);
DECLS(Voice_state);
DECLS(Work_buffer);
DECLS(Work_buffers);

#undef DECLS


#endif // KQT_DECL_H


