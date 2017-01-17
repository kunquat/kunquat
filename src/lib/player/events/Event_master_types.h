

/*
 * Author: Tomi Jylhä-Ollila, Finland 2013-2017
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#ifndef EVENT_MASTER_DEF
#error "EVENT_MASTER_DEF(type) not defined"
#endif


EVENT_MASTER_DEF("mpd",     pattern_delay,              TSTAMP,         v_nonneg_ts)

EVENT_MASTER_DEF("m.jc",    set_jump_counter,           INT,            v_counter)
EVENT_MASTER_DEF("m.jr",    set_jump_row,               TSTAMP,         v_nonneg_ts)
EVENT_MASTER_DEF("m.jp",    set_jump_pat_inst,          PAT_INST_REF,   v_piref)
EVENT_MASTER_DEF("mj",      jump,                       NONE,           NULL)

EVENT_MASTER_DEF("m.t",     set_tempo,                  FLOAT,          v_tempo)
EVENT_MASTER_DEF("m/t",     slide_tempo,                FLOAT,          v_tempo)
EVENT_MASTER_DEF("m/=t",    slide_tempo_length,         TSTAMP,         v_nonneg_ts)

EVENT_MASTER_DEF("m.r",     set_retuner,                INT,            v_tuning_table)
EVENT_MASTER_DEF("m.rfp",   set_retuner_fixed_pitch,    FLOAT,          v_pitch)
EVENT_MASTER_DEF("m.rc",    set_retuner_tuning_centre,  FLOAT,          v_pitch)
EVENT_MASTER_DEF("m.ro",    set_retuner_pitch_offset,   FLOAT,          v_finite_float)
EVENT_MASTER_DEF("mmr",     mimic_retuner,              INT,            v_tuning_table)
EVENT_MASTER_DEF("m<r",     reset_retuner,              NONE,           NULL)

EVENT_MASTER_DEF("m.v",     set_volume,                 FLOAT,          v_volume)
EVENT_MASTER_DEF("m/v",     slide_volume,               FLOAT,          v_volume)
EVENT_MASTER_DEF("m/=v",    slide_volume_length,        TSTAMP,         v_nonneg_ts)


#undef EVENT_MASTER_DEF


