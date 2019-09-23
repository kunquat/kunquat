

/*
 * Author: Tomi Jylhä-Ollila, Finland 2016-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <player/devices/processors/Ks_state.h>

#include <debug/assert.h>
#include <init/devices/Device.h>
#include <init/devices/processors/Proc_ks.h>
#include <intrinsics.h>
#include <mathnum/common.h>
#include <mathnum/conversions.h>
#include <mathnum/Random.h>
#include <player/devices/Device_thread_state.h>
#include <player/devices/processors/Filter.h>
#include <player/devices/processors/Proc_state_utils.h>
#include <player/devices/Voice_state.h>
#include <player/Work_buffer.h>
#include <player/Work_buffers.h>

#include <math.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>


#define DAMP_FILTER_ORDER 1


typedef struct Damp_state
{
    double cutoff_norm; // used for calculating phase delay
    double coeffs[DAMP_FILTER_ORDER];
    double mul;
    double history1[DAMP_FILTER_ORDER];
    double history2[DAMP_FILTER_ORDER];
} Damp_state;


typedef struct Frac_delay
{
    float eta;
    float prev_item;
    float feedback;
} Frac_delay;


typedef struct Read_state
{
    int32_t read_pos;
    float pitch;
    float damp;
    Damp_state damp_state;
    Frac_delay frac_delay;
} Read_state;


static void Read_state_clear(Read_state* rs)
{
    rassert(rs != NULL);

    rs->read_pos = 0;
    rs->pitch = NAN;
    rs->damp = NAN;

    rs->damp_state.mul = 0;

    for (int i = 0; i < DAMP_FILTER_ORDER; ++i)
    {
        rs->damp_state.coeffs[i] = 0;
        rs->damp_state.history1[i] = 0;
        rs->damp_state.history2[i] = 0;
    }

    rs->frac_delay.eta = 0;
    rs->frac_delay.prev_item = 0;
    rs->frac_delay.feedback = 0;

    return;
}


#define CUTOFF_BIAS 74.37631656229591


static void Read_state_modify(
        Read_state* rs,
        float pitch,
        float damp,
        int32_t write_pos,
        int32_t buf_len,
        int32_t audio_rate)
{
    rassert(rs != NULL);
    rassert(isfinite(pitch));
    rassert(damp >= KS_MIN_DAMP);
    rassert(damp <= KS_MAX_DAMP);
    rassert(write_pos >= 0);
    rassert(buf_len > 2);
    rassert(write_pos < buf_len);
    rassert(audio_rate > 0);

    const double freq = cents_to_Hz(pitch);
    const double period_length = audio_rate / freq;
    const double used_buf_length = clamp(period_length, 2, buf_len);
    const int32_t used_buf_frames_whole = (int32_t)floor(used_buf_length);

    if (isnan(rs->damp) || fabs(rs->damp - damp) > 0.001)
    {
        // Set up damping filter
        const double cutoff = exp2((100 - damp + CUTOFF_BIAS) / 12.0);
        const double cutoff_clamped = clamp(cutoff, 1, (audio_rate / 2) - 1);
        rs->damp_state.cutoff_norm = cutoff_clamped / audio_rate;
        one_pole_filter_create(
                rs->damp_state.cutoff_norm,
                0,
                rs->damp_state.coeffs,
                &rs->damp_state.mul);
    }

    rs->read_pos = (buf_len + write_pos - used_buf_frames_whole) % buf_len;
    rs->pitch = pitch;
    rs->damp = damp;

    // Phase delay for one-pole lowpass is: atan(tan(pi*f)/tan(pi*f0))/(2*pi*f)
    const double freq_norm = freq / audio_rate;
    const double delay_add =
        -atan(tan(PI * freq_norm)/tan(PI * rs->damp_state.cutoff_norm)) /
        (2 * PI * freq_norm);

    // Set up fractional delay filter
    float delay = (float)(used_buf_length - used_buf_frames_whole) + (float)delay_add;
    // Set delay to range [0.618, 1.618) to minimise clicking as suggested by
    // Van Duyne et al.: A Lossless, Click-free, Pitchbend-able Delay Line Loop
    // Interpolation Scheme
    while (delay < 0.618f)
    {
        delay += 1.0f;
        rs->read_pos = (rs->read_pos + 1) % buf_len;
    }

    rs->frac_delay.eta = (1 - delay) / (1 + delay);

    return;
}


static void Read_state_init(
        Read_state* rs,
        float damp,
        float pitch,
        int32_t write_pos,
        int32_t buf_len,
        int32_t audio_rate)
{
    rassert(rs != NULL);
    rassert(damp >= KS_MIN_DAMP);
    rassert(damp <= KS_MAX_DAMP);
    rassert(isfinite(pitch));
    rassert(write_pos >= 0);
    rassert(buf_len > 2);
    rassert(write_pos < buf_len);
    rassert(audio_rate > 0);

    Read_state_clear(rs);

    Read_state_modify(rs, pitch, damp, write_pos, buf_len, audio_rate);

    return;
}


static float Read_state_update(
        Read_state* rs,
        float excitation,
        int32_t delay_buf_len,
        const float delay_buf[delay_buf_len])
{
    rassert(rs != NULL);
    rassert(delay_buf_len > 2);
    rassert(delay_buf != NULL);

    const float src_value = excitation + delay_buf[rs->read_pos];

    // Apply damping filter
    double damped = nq_zero_filter(
            DAMP_FILTER_ORDER, rs->damp_state.history1, src_value);
    damped = iir_filter_strict_cascade(
            DAMP_FILTER_ORDER, rs->damp_state.coeffs, rs->damp_state.history2, damped);
    damped *= rs->damp_state.mul;

    // Apply fractional delay filter
    // Based on the description in
    // https://ccrma.stanford.edu/~jos/Interpolation/Allpass_Interpolated_Delay_Line.html
    Frac_delay* fd = &rs->frac_delay;
    const float value =
        fd->eta * (float)damped + fd->prev_item - fd->eta * fd->feedback;
    fd->prev_item = (float)damped;
    fd->feedback = value;

    ++rs->read_pos;
    if (rs->read_pos >= delay_buf_len)
        rs->read_pos = 0;

    return value;
}


static void Read_state_copy(Read_state* restrict dest, const Read_state* restrict src)
{
    rassert(dest != NULL);
    rassert(src != NULL);

    *dest = *src;

    return;
}


#define SINC_WINDOW_EXTENT 8
#define RESAMPLE_HISTORY_SIZE (SINC_WINDOW_EXTENT * 2)


typedef struct Resample_state
{
    // Configuration
    const Work_buffer* from_wb;
    Work_buffer* to_wb;
    int32_t from_rate;
    int32_t to_rate;

    // State
    _Alignas(64) float in_history[RESAMPLE_HISTORY_SIZE];
    int32_t from_index;
    int32_t sub_phase;
    int32_t ds_sub_phase;
    int32_t to_index;
} Resample_state;


static void Resample_state_init(
        Resample_state* state, int32_t from_rate, int32_t to_rate)
{
    rassert(state != NULL);
    rassert(from_rate > 0);
    rassert(to_rate > 0);

    state->from_wb = NULL;
    state->to_wb = NULL;
    state->from_rate = from_rate;
    state->to_rate = to_rate;

    for (int i = 0; i < RESAMPLE_HISTORY_SIZE; ++i)
        state->in_history[i] = 0;

    state->from_index = 0;
    state->sub_phase = 0;
    state->ds_sub_phase = 0;
    state->to_index = 0;

    return;
}


static void Resample_state_prepare_render(
        Resample_state* state, const Work_buffer* from_wb, Work_buffer* to_wb)
{
    rassert(state != NULL);

    state->from_wb = from_wb;
    state->to_wb = to_wb;
    state->from_index = 0;
    state->to_index = 0;

    return;
}


#define USE_SINC 1
#if USE_SINC

#define USE_SSE_SINC KQT_SSE

#if USE_SSE_SINC

static_assert(RESAMPLE_HISTORY_SIZE % 4 == 0,
        "RESAMPLE_HISTORY_SIZE is incompatible with the SSE sinc implementation.");

static float make_sinc_item(float history[RESAMPLE_HISTORY_SIZE], float shift_rem)
{
    dassert(history != NULL);
    dassert(shift_rem > 0);

    const __m128 shift_rem_adds =
        _mm_set_ps(3 - shift_rem, 2 - shift_rem, 1 - shift_rem, -shift_rem);
    int shift_floor = -SINC_WINDOW_EXTENT + 1;
    float rep_sin_shift = sinf(((float)shift_floor - shift_rem) * (float)PI);
    const __m128 rep_sin_shifts =
        _mm_set_ps(-rep_sin_shift, rep_sin_shift, -rep_sin_shift, rep_sin_shift);

    const __m128 window_scale = _mm_set1_ps(1.0f / (float)SINC_WINDOW_EXTENT);
    const __m128 pi = _mm_set1_ps((float)PI);

    __m128 results = _mm_set1_ps(0);

    for (int i = 0; i < RESAMPLE_HISTORY_SIZE; i += 4)
    {
        const __m128 shifts =
            _mm_add_ps(_mm_set1_ps((float)shift_floor), shift_rem_adds);

        const __m128 w = _mm_mul_ps(shifts, window_scale);
        const __m128 w2 = _mm_mul_ps(w, w);
        const __m128 w4 = _mm_mul_ps(w2, w2);
        const __m128 part4 = _mm_mul_ps(_mm_set1_ps(0.5f), w4);
        const __m128 part2 =
            _mm_mul_ps(_mm_set1_ps(1.5f), _mm_sub_ps(_mm_set1_ps(1.0f), w2));
        const __m128 window = _mm_add_ps(_mm_add_ps(part4, part2), _mm_set1_ps(-0.5f));

        const __m128 items = _mm_load_ps(history + i);
        const __m128 add = _mm_mul_ps(
                _mm_mul_ps(_mm_div_ps(rep_sin_shifts, _mm_mul_ps(shifts, pi)), window),
                items);

        results = _mm_add_ps(results, add);

        shift_floor += 4;
    }

    float ra[4];
    _mm_store_ps(ra, results);
    const float result = ra[0] + ra[1] + ra[2] + ra[3];
    return result;
}

#else

#if 0
static float sinc_norm(float x)
{
    dassert(x != 0);
    const float scaled_x = x * (float)PI;
    return sinf(scaled_x) / scaled_x;
}
#endif

static float make_sinc_item(float history[RESAMPLE_HISTORY_SIZE], float shift_rem)
{
    dassert(history != NULL);
    dassert(shift_rem > 0);

    int8_t shift_floor = -SINC_WINDOW_EXTENT + 1;
    float rep_sin_shift = sinf((shift_floor - shift_rem) * (float)PI);

    float result = 0;
    for (int i = 0; i < RESAMPLE_HISTORY_SIZE; ++i)
    {
        const float shift = shift_floor - shift_rem;
        const float w = shift / SINC_WINDOW_EXTENT;
        const float w2 = w * w;
        const float w4 = w2 * w2;
        const float window = 0.5f * w4 + 1.5f * (1 - w2) - 0.5f;
        //const float window = sinc_norm(shift / SINC_WINDOW_EXTENT);
        const float add = (rep_sin_shift / (shift * (float)PI)) * window * history[i];

        rep_sin_shift = -rep_sin_shift;
        result += add;
        ++shift_floor;
    }

    return result;
}

#endif // USE_SSE_SINC

#endif // USE_SINC


static void Resample_state_process(
        Resample_state* state, int32_t req_input_count, int32_t req_output_count)
{
    rassert(state != NULL);
    rassert(state->from_wb != NULL);
    rassert(state->to_wb != NULL);
    rassert(state->from_rate > 0);
    rassert(state->to_rate > 0);
    rassert(state->from_rate != state->to_rate);
    rassert(req_input_count >= 0);
    rassert(req_output_count > 0);

    const int32_t sub_phase_div = max(state->from_rate, state->to_rate);
    const int32_t sub_phase_add = min(state->from_rate, state->to_rate);

    int32_t from_index = state->from_index;
    const int32_t from_size = Work_buffer_get_size(state->from_wb);

    int32_t sub_phase = state->sub_phase;

    int32_t to_index = state->to_index;
    const int32_t to_size = Work_buffer_get_size(state->to_wb);

    const float* from = Work_buffer_get_contents(state->from_wb);
    float* to = Work_buffer_get_contents_mut(state->to_wb);

    float* in_history = state->in_history;

#if USE_SINC
    if (state->to_rate < state->from_rate)
    {
        int32_t ds_sub_phase = state->ds_sub_phase;
        const int32_t ds_sub_phase_div = min(state->from_rate, state->to_rate);
        const int32_t ds_sub_phase_add = max(state->from_rate, state->to_rate);

        // Downsample
        rassert(req_input_count > 0);
        const int32_t max_input_count = from_size - from_index;
        rassert(max_input_count > 0);
        const int32_t actual_input_count = min(req_input_count, max_input_count);
        const int32_t output_stop = (req_output_count < INT32_MAX - to_index)
            ? to_index + req_output_count : INT32_MAX;

        for (int32_t i = 0; i < actual_input_count; ++i)
        {
            const int last_history_index = RESAMPLE_HISTORY_SIZE - 1;
            for (int k = 0; k < last_history_index; ++k)
                in_history[k] = in_history[k + 1];
            in_history[last_history_index] = from[from_index];

            ++from_index;

            sub_phase += sub_phase_add;
            if (sub_phase >= sub_phase_div)
            {
                ds_sub_phase = (ds_sub_phase + ds_sub_phase_add) % ds_sub_phase_div;

                if (to_index >= output_stop)
                {
                    sub_phase -= sub_phase_add;
                    ds_sub_phase = (ds_sub_phase + ds_sub_phase_div - ds_sub_phase_add) %
                        ds_sub_phase_div;
                    break;
                }

                sub_phase -= sub_phase_div;

                if (sub_phase > 0)
                    to[to_index] = make_sinc_item(
                            in_history, ((float)ds_sub_phase / (float)ds_sub_phase_div));
                else
                    to[to_index] = in_history[SINC_WINDOW_EXTENT];

                ++to_index;
            }
        }

        state->ds_sub_phase = ds_sub_phase;
    }
    else
    {
        // Upsample
        const int32_t max_output_count = to_size - to_index;
        rassert(max_output_count > 0);
        const int32_t actual_output_count = min(req_output_count, max_output_count);
        const int32_t input_stop = (req_input_count < INT32_MAX - from_index)
            ? from_index + req_input_count : INT32_MAX;

        for (int32_t i = 0; i < actual_output_count; ++i)
        {
            sub_phase += sub_phase_add;
            if (sub_phase >= sub_phase_div)
            {
                if (from_index >= input_stop)
                {
                    sub_phase -= sub_phase_add;
                    break;
                }

                sub_phase -= sub_phase_div;

                const int last_history_index = RESAMPLE_HISTORY_SIZE - 1;
                for (int k = 0; k < last_history_index; ++k)
                    in_history[k] = in_history[k + 1];
                in_history[last_history_index] = from[from_index];

                ++from_index;
            }

            if (sub_phase > 0)
                to[to_index] = make_sinc_item(
                        in_history, (float)sub_phase / (float)sub_phase_div);
            else
                to[to_index] = in_history[SINC_WINDOW_EXTENT - 1];

            ++to_index;
        }
    }
#else
    if (state->to_rate < state->from_rate)
    {
        int32_t ds_sub_phase = state->ds_sub_phase;
        const int32_t ds_sub_phase_div = min(state->from_rate, state->to_rate);
        const int32_t ds_sub_phase_add = max(state->from_rate, state->to_rate);

        // Downsample
        rassert(req_input_count > 0);
        const int32_t max_input_count = from_size - from_index;
        rassert(max_input_count > 0);
        const int32_t actual_input_count = min(req_input_count, max_input_count);
        const int32_t output_stop = (req_output_count < INT32_MAX - to_index)
            ? to_index + req_output_count : INT32_MAX;

        for (int32_t i = 0; i < actual_input_count; ++i)
        {
            in_history[0] = in_history[1];
            in_history[1] = from[from_index];

            ++from_index;

            sub_phase += sub_phase_add;
            if (sub_phase >= sub_phase_div)
            {
                ds_sub_phase = (ds_sub_phase + ds_sub_phase_add) % ds_sub_phase_div;

                if (to_index >= output_stop)
                {
                    sub_phase -= sub_phase_add;
                    ds_sub_phase = (ds_sub_phase + ds_sub_phase_div - ds_sub_phase_add) %
                        ds_sub_phase_div;
                    break;
                }

                sub_phase -= sub_phase_div;
                const float lerp_t = ((float)ds_sub_phase / (float)ds_sub_phase_div);
                to[to_index] = lerp(in_history[0], in_history[1], lerp_t);

                ++to_index;
            }
        }

        state->ds_sub_phase = ds_sub_phase;
    }
    else
    {
        // Upsample
        const int32_t max_output_count = to_size - to_index;
        rassert(max_output_count > 0);
        const int32_t actual_output_count = min(req_output_count, max_output_count);
        const int32_t input_stop = (req_input_count < INT32_MAX - from_index)
            ? from_index + req_input_count : INT32_MAX;

        for (int32_t i = 0; i < actual_output_count; ++i)
        {
            sub_phase += sub_phase_add;
            if (sub_phase >= sub_phase_div)
            {
                if (from_index >= input_stop)
                {
                    sub_phase -= sub_phase_add;
                    break;
                }

                sub_phase -= sub_phase_div;

                in_history[0] = in_history[1];
                in_history[1] = from[from_index];

                ++from_index;
            }

            const float lerp_t = (float)sub_phase / (float)sub_phase_div;
            to[to_index] = lerp(in_history[0], in_history[1], lerp_t);

            ++to_index;
        }
    }
#endif

    state->from_index = from_index;
    state->sub_phase = sub_phase;
    state->to_index = to_index;

    return;
}


#define RESAMPLE_LP_ORDER 6


typedef struct Resample_lp_state
{
    double coeffs[RESAMPLE_LP_ORDER];
    double mul;
    double history1[RESAMPLE_LP_ORDER];
    double history2[RESAMPLE_LP_ORDER];
} Resample_lp_state;


typedef struct Ks_vstate
{
    Voice_state parent;

    int32_t write_pos;
    int primary_read_state;
    bool is_xfading;
    double xfade_progress;
    Read_state read_states[2];
    Resample_lp_state resample_lp_state;
    Resample_state excit_resample_state;
    Resample_state output_resample_state;
} Ks_vstate;


int32_t Ks_vstate_get_size(void)
{
    return sizeof(Ks_vstate);
}


static int32_t get_next_xfade_start_index(
        const Work_buffer* pitches_wb,
        const float damps[],
        const Read_state* read_state,
        int32_t search_start_index,
        int32_t frame_count,
        float* next_pitch,
        float* next_damp)
{
    rassert(pitches_wb != NULL);
    rassert(damps != NULL);
    rassert(read_state != NULL);
    rassert(search_start_index >= 0);
    rassert(frame_count > 0);
    rassert(next_pitch != NULL);
    rassert(next_damp != NULL);

    if (search_start_index >= frame_count)
        return frame_count;

    const int32_t const_pitch_start = Work_buffer_get_const_start(pitches_wb);
    const float* pitches = Work_buffer_get_contents(pitches_wb);

    const int32_t pitch_var_stop = min(const_pitch_start, frame_count);

    const float cur_pitch = read_state->pitch;
    const float cur_damp = read_state->damp;

    {
        const float max_pitch_diff = 0.1f;
        const float max_damp_diff = 0.001f;

        for (int32_t i = search_start_index; i < pitch_var_stop; ++i)
        {
            const float pitch = pitches[i];
            const float damp = damps[i];
            const float clamped_damp = clamp(damp, KS_MIN_DAMP, KS_MAX_DAMP);

            if ((fabs(pitch - cur_pitch) > max_pitch_diff) ||
                    (fabs(clamped_damp - cur_damp) > max_damp_diff))
            {
                *next_pitch = pitch;
                *next_damp = clamped_damp;
                return i;
            }
        }
    }

    if (pitch_var_stop < frame_count)
    {
        const float pitch = pitches[pitch_var_stop];

        const float max_pitch_diff = 0.001f;
        if (fabs(pitch - cur_pitch) > max_pitch_diff)
        {
            const int32_t index = max(pitch_var_stop, search_start_index);
            *next_pitch = pitch;
            const float damp = damps[index];
            *next_damp = clamp(damp, KS_MIN_DAMP, KS_MAX_DAMP);
            return index;
        }

        const float max_damp_diff = 0.001f;

        for (int32_t i = max(pitch_var_stop, search_start_index); i < frame_count; ++i)
        {
            const float damp = damps[i];
            const float clamped_damp = clamp(damp, KS_MIN_DAMP, KS_MAX_DAMP);

            if (fabs(clamped_damp - cur_damp) > max_damp_diff)
            {
                *next_pitch = pitch;
                *next_damp = clamped_damp;
                return i;
            }
        }
    }

    return frame_count;
}


enum
{
    PORT_IN_PITCH = 0,
    PORT_IN_FORCE,
    PORT_IN_EXCITATION,
    PORT_IN_DAMP,
    PORT_IN_COUNT
};


enum
{
    PORT_OUT_AUDIO = 0,
    PORT_OUT_COUNT
};


static const int KS_WB_FIXED_PITCH          = WORK_BUFFER_IMPL_1;
static const int KS_WB_FIXED_FORCE          = WORK_BUFFER_IMPL_2;
static const int KS_WB_FIXED_EXCITATION     = WORK_BUFFER_IMPL_3;
static const int KS_WB_FIXED_DAMP           = WORK_BUFFER_IMPL_4;
static const int KS_WB_FILTERED_EXCITATION  = WORK_BUFFER_IMPL_5;
static const int KS_WB_RES_EXCITATION       = WORK_BUFFER_IMPL_6;
static const int KS_WB_RES_OUTPUT           = WORK_BUFFER_IMPL_7;


int32_t Ks_vstate_render_voice(
        Voice_state* vstate,
        Proc_state* proc_state,
        const Device_thread_state* proc_ts,
        const Au_state* au_state,
        const Work_buffers* wbs,
        int32_t frame_count,
        double tempo)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);
    rassert(proc_ts != NULL);
    rassert(au_state != NULL);
    rassert(wbs != NULL);
    rassert(frame_count > 0);
    rassert(tempo > 0);

    const Proc_ks* ks = (const Proc_ks*)proc_state->parent.device->dimpl;

    Ks_vstate* ks_vstate = (Ks_vstate*)
#ifdef __GNUC__
        __builtin_assume_aligned(
#endif
        vstate
#ifdef __GNUC__
        , VOICE_STATE_ALIGNMENT)
#endif
        ;

    const int32_t system_audio_rate = ks_vstate->excit_resample_state.from_rate;
    const int32_t ks_audio_rate = ks_vstate->excit_resample_state.to_rate;
    const bool resampling_needed = (system_audio_rate != ks_audio_rate);

    // Get output buffer for writing
    Work_buffer* final_out_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_SEND, PORT_OUT_AUDIO);
    if (final_out_wb == NULL)
        return 0;
    float* final_out_buf = Work_buffer_get_contents_mut(final_out_wb);
    float* out_buf = final_out_buf;

    // Get volume scales
    Work_buffer* scales_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_FORCE);
    Work_buffer* dBs_wb = scales_wb;
    if (Work_buffer_is_valid(dBs_wb) &&
            Work_buffer_is_final(dBs_wb) &&
            (Work_buffer_get_const_start(dBs_wb) == 0) &&
            (Work_buffer_get_contents(dBs_wb)[0] == -INFINITY))
    {
        // We are only getting silent force from this point onwards
        vstate->active = false;
        return 0;
    }

    // Get frequencies
    const Work_buffer* pitches_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_PITCH);
    if (!Work_buffer_is_valid(pitches_wb))
    {
        Work_buffer* fixed_pitches_wb =
            Work_buffers_get_buffer_mut(wbs, KS_WB_FIXED_PITCH);
        Work_buffer_clear(fixed_pitches_wb, 0, frame_count);
        pitches_wb = fixed_pitches_wb;
    }
    const float* pitches = Work_buffer_get_contents(pitches_wb);

    if (!Work_buffer_is_valid(scales_wb))
        scales_wb = Work_buffers_get_buffer_mut(wbs, KS_WB_FIXED_FORCE);
    Proc_fill_scale_buffer(scales_wb, dBs_wb, frame_count);
    const float* scales = Work_buffer_get_contents(scales_wb);

    // Get excitation signal
    Work_buffer* src_excit_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_EXCITATION);
    if (!Work_buffer_is_valid(src_excit_wb))
    {
        Work_buffer* fixed_excit_wb =
            Work_buffers_get_buffer_mut(wbs, KS_WB_FIXED_EXCITATION);
        Work_buffer_clear(fixed_excit_wb, 0, frame_count);
        src_excit_wb = fixed_excit_wb;
    }
    const float* src_excits = Work_buffer_get_contents(src_excit_wb);
    const float* excits = src_excits;

    // Get resampling buffers if needed
    Work_buffer* filtered_excit_wb = NULL;
    Work_buffer* res_excit_wb = NULL;
    Work_buffer* res_out_wb = NULL;
    if (resampling_needed)
    {
        filtered_excit_wb = Work_buffers_get_buffer_mut(wbs, KS_WB_FILTERED_EXCITATION);
        res_excit_wb = Work_buffers_get_buffer_mut(wbs, KS_WB_RES_EXCITATION);
        res_out_wb = Work_buffers_get_buffer_mut(wbs, KS_WB_RES_OUTPUT);
        excits = Work_buffer_get_contents(res_excit_wb);
        out_buf = Work_buffer_get_contents_mut(res_out_wb);
    }

    // Get damp signal
    const Work_buffer* damps_wb = Device_thread_state_get_voice_buffer(
            proc_ts, DEVICE_PORT_TYPE_RECV, PORT_IN_DAMP);
    if (!Work_buffer_is_valid(damps_wb))
    {
        Work_buffer* fixed_damps_wb = Work_buffers_get_buffer_mut(wbs, KS_WB_FIXED_DAMP);
        float* damps = Work_buffer_get_contents_mut(fixed_damps_wb);

        const float fixed_damp = (float)ks->damp;
        for (int32_t i = 0; i < frame_count; ++i)
            damps[i] = fixed_damp;

        Work_buffer_set_const_start(fixed_damps_wb, 0);
        damps_wb = fixed_damps_wb;
    }
    const float* damps = Work_buffer_get_contents(damps_wb);

    int32_t write_pos = ks_vstate->write_pos;
    int primary_index = ks_vstate->primary_read_state;
    bool is_xfading = ks_vstate->is_xfading;
    double xfade_progress = ks_vstate->xfade_progress;

    Read_state* primary_rs = &ks_vstate->read_states[primary_index];
    Read_state* other_rs = &ks_vstate->read_states[1 - primary_index];

    // Get delay buffer
    Work_buffer* delay_wb = ks_vstate->parent.wb;
    rassert(delay_wb != NULL);
    const int32_t delay_wb_size = Work_buffer_get_size(delay_wb);

    const bool need_init = isnan(primary_rs->pitch);

    if (need_init)
    {
        write_pos = 0;

        Read_state_init(
                primary_rs,
                clamp(damps[0], KS_MIN_DAMP, KS_MAX_DAMP),
                pitches[0],
                write_pos,
                delay_wb_size,
                ks_audio_rate);
    }

    float* delay_buf = Work_buffer_get_contents_mut(delay_wb);

    const double xfade_speed = 1000.0;
    const double xfade_step = xfade_speed / ks_audio_rate;
    const int32_t sys_xfade_time = (int32_t)ceil(system_audio_rate / xfade_speed);

    int32_t next_sys_xfade_start_index = 0;
    int32_t next_ks_xfade_start_index = 0;
    float next_pitch = primary_rs->pitch;
    float next_damp = primary_rs->damp;

    int32_t xfade_check_start = 0;

    if (is_xfading)
    {
        xfade_check_start =
            (int32_t)ceil((1 - xfade_progress) * system_audio_rate / xfade_speed);
    }
    else
    {
        next_sys_xfade_start_index = get_next_xfade_start_index(
                pitches_wb,
                damps,
                primary_rs,
                xfade_check_start,
                frame_count,
                &next_pitch,
                &next_damp);

        xfade_check_start = next_sys_xfade_start_index + sys_xfade_time;

        next_ks_xfade_start_index = next_sys_xfade_start_index;
        if (resampling_needed)
            next_ks_xfade_start_index = (int32_t)(
                    next_sys_xfade_start_index *
                    ks_audio_rate /
                    (double)system_audio_rate);
    }

    if (resampling_needed)
    {
        const Work_buffer* excit_wb =
            (ks_audio_rate < system_audio_rate) ? filtered_excit_wb : src_excit_wb;
        Resample_state_prepare_render(
                &ks_vstate->excit_resample_state, excit_wb, res_excit_wb);
        Resample_state_prepare_render(
                &ks_vstate->output_resample_state, res_out_wb, final_out_wb);
    }

    int32_t sys_frames_processed = 0;
    int32_t ks_frames_processed = 0;

    while (sys_frames_processed < frame_count)
    {
        int32_t cur_sys_frame_count = frame_count - sys_frames_processed;
        int32_t cur_ks_frame_count = cur_sys_frame_count;
        if (resampling_needed)
        {
            // Filter input if downsampling
            if (ks_audio_rate < system_audio_rate)
            {
                const float* orig_excits = Work_buffer_get_contents(src_excit_wb);
                float* filtered_excits = Work_buffer_get_contents_mut(filtered_excit_wb);
                Resample_lp_state* rls = &ks_vstate->resample_lp_state;

                for (int32_t i = 0; i < cur_sys_frame_count; ++i)
                {
                    const float orig_value = orig_excits[i];

                    double result =
                        nq_zero_filter(RESAMPLE_LP_ORDER, rls->history1, orig_value);
                    result = iir_filter_strict_cascade_even_order(
                            RESAMPLE_LP_ORDER, rls->coeffs, rls->history2, result);
                    result *= rls->mul;

                    filtered_excits[i] = (float)result;
                }
            }

            Resample_state* ers = &ks_vstate->excit_resample_state;

            // We need to write at the beginning of target buffer every time
            ers->to_index = 0;

            const int32_t prev_from = ers->from_index;

            Resample_state_process(
                    ers, cur_sys_frame_count, Work_buffer_get_size(res_excit_wb));

            cur_sys_frame_count = ers->from_index - prev_from;
            cur_ks_frame_count = ers->to_index;
        }

        for (int32_t i = 0; i < cur_ks_frame_count; ++i)
        {
            const float excitation = excits[i];

            if (!is_xfading)
            {
                if (ks_frames_processed + i >= next_ks_xfade_start_index)
                {
                    // Instantaneous slides to lower pitches don't work very well,
                    // so limit the step length when sliding downwards
                    const float min_pitch = primary_rs->pitch - 200.0f;
                    const float cur_target_pitch = max(min_pitch, next_pitch);

                    Read_state_copy(other_rs, primary_rs);
                    Read_state_modify(
                            other_rs,
                            cur_target_pitch,
                            next_damp,
                            write_pos,
                            delay_wb_size,
                            ks_audio_rate);

                    primary_index = 1 - primary_index;
                    primary_rs = &ks_vstate->read_states[primary_index];
                    other_rs = &ks_vstate->read_states[1 - primary_index];

                    is_xfading = true;
                    xfade_progress = 0.0;
                }
            }

            float value = 0.0f;
            if (!is_xfading)
            {
                value = Read_state_update(
                        primary_rs, excitation, delay_wb_size, delay_buf);
            }
            else
            {
                const float out_value =
                    Read_state_update(other_rs, excitation, delay_wb_size, delay_buf);

                const float in_value =
                    Read_state_update(primary_rs, excitation, delay_wb_size, delay_buf);

                value = lerp(out_value, in_value, (float)xfade_progress);

                xfade_progress += xfade_step;
                if (xfade_progress >= 1.0)
                {
                    is_xfading = false;

                    // Preserve backwards compatibility
                    if (!resampling_needed)
                        xfade_check_start = i + 1;

                    next_sys_xfade_start_index = get_next_xfade_start_index(
                            pitches_wb,
                            damps,
                            primary_rs,
                            xfade_check_start,
                            frame_count,
                            &next_pitch,
                            &next_damp);

                    xfade_check_start = next_sys_xfade_start_index + sys_xfade_time;

                    next_ks_xfade_start_index = next_sys_xfade_start_index;
                    if (resampling_needed)
                        next_ks_xfade_start_index = (int32_t)(
                                next_sys_xfade_start_index *
                                ks_audio_rate /
                                (double)system_audio_rate);
                }
            }

            out_buf[i] = value;

            delay_buf[write_pos] = value;

            ++write_pos;
            if (write_pos >= delay_wb_size)
                write_pos = 0;
        }

        if (resampling_needed)
        {
            Resample_state* ors = &ks_vstate->output_resample_state;

            // We need to read from the beginning of the source buffer every time
            ors->from_index = 0;

            Resample_state_process(ors, cur_ks_frame_count, cur_sys_frame_count);
        }

        sys_frames_processed += cur_sys_frame_count;
        ks_frames_processed += cur_ks_frame_count;
    }

    for (int32_t i = 0; i < frame_count; ++i)
        final_out_buf[i] *= scales[i];

    ks_vstate->write_pos = write_pos;
    ks_vstate->primary_read_state = primary_index;
    ks_vstate->is_xfading = is_xfading;
    ks_vstate->xfade_progress = xfade_progress;

    return frame_count;
}


void Ks_vstate_init(Voice_state* vstate, const Proc_state* proc_state)
{
    rassert(vstate != NULL);
    rassert(proc_state != NULL);

    Ks_vstate* ks_vstate = (Ks_vstate*)
#ifdef __GNUC__
        __builtin_assume_aligned(
#endif
        vstate
#ifdef __GNUC__
        , VOICE_STATE_ALIGNMENT)
#endif
        ;

    ks_vstate->write_pos = 0;
    ks_vstate->primary_read_state = 0;
    ks_vstate->is_xfading = false;
    ks_vstate->xfade_progress = 0.0;

    Read_state_clear(&ks_vstate->read_states[0]);
    Read_state_clear(&ks_vstate->read_states[1]);

    // Init resampling
    {
        const Proc_ks* ks = (const Proc_ks*)proc_state->parent.device->dimpl;

        const int32_t system_audio_rate = proc_state->parent.audio_rate;
        int32_t ks_audio_rate = system_audio_rate;
        if (ks->audio_rate_range_enabled)
        {
            const int32_t max_rate =
                max(ks->audio_rate_range_min, ks->audio_rate_range_max);
            ks_audio_rate = clamp(system_audio_rate, ks->audio_rate_range_min, max_rate);
        }

        if (ks_audio_rate < system_audio_rate)
        {
            Resample_lp_state* rls = &ks_vstate->resample_lp_state;

            const double lp_freq = ks_audio_rate * 0.47 / (double)system_audio_rate;
            butterworth_filter_create(
                    RESAMPLE_LP_ORDER, lp_freq, 0, rls->coeffs, &rls->mul);

            for (int i = 0; i < RESAMPLE_LP_ORDER; ++i)
            {
                rls->history1[i] = 0;
                rls->history2[i] = 0;
            }
        }

        Resample_state_init(
                &ks_vstate->excit_resample_state, system_audio_rate, ks_audio_rate);
        Resample_state_init(
                &ks_vstate->output_resample_state, ks_audio_rate, system_audio_rate);
    }

    Work_buffer* delay_wb = ks_vstate->parent.wb;
    rassert(delay_wb != NULL);
    Work_buffer_clear(delay_wb, 0, Work_buffer_get_size(delay_wb));

    return;
}


