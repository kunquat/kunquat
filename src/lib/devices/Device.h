

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


#ifndef K_DEVICE_H
#define K_DEVICE_H


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <Audio_buffer.h>
#include <Decl.h>
#include <devices/Device_params.h>
#include <frame.h>
#include <kunquat/limits.h>
#include <mathnum/Random.h>
#include <player/Device_states.h>
#include <player/Linear_controls.h>
#include <player/Work_buffers.h>
#include <string/Streader.h>
#include <Tstamp.h>


typedef enum
{
    DEVICE_CONTROL_VAR_MODE_MIXED,
    DEVICE_CONTROL_VAR_MODE_VOICE,
} Device_control_var_mode;


typedef Device_state* Device_create_state_func(
        const Device*, int32_t buffer_size, int32_t audio_rate);


typedef void Device_process_signal_func(
        const Device*,
        Device_states*,
        const Work_buffers*,
        uint32_t buf_start,
        uint32_t buf_stop,
        uint32_t audio_rate,
        double tempo);


typedef void Device_set_control_var_generic_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Random*,
        Channel*,
        const char* var_name,
        const Value* value);

typedef void Device_slide_control_var_float_target_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Channel*,
        const char* var_name,
        double value);

typedef void Device_slide_control_var_float_length_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Channel*,
        const char* var_name,
        const Tstamp* length);

typedef void Device_osc_speed_cv_float_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Channel*,
        const char* var_name,
        double speed);

typedef void Device_osc_depth_cv_float_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Channel*,
        const char* var_name,
        double depth);

typedef void Device_osc_speed_slide_cv_float_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Channel*,
        const char* var_name,
        const Tstamp* length);

typedef void Device_osc_depth_slide_cv_float_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Channel*,
        const char* var_name,
        const Tstamp* length);

typedef void Device_init_control_vars_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Random*,
        Channel*);

typedef void Device_init_control_var_float_func(
        const Device*,
        Device_states*,
        Device_control_var_mode,
        Channel*,
        const char*,
        const Linear_controls*);


struct Device
{
    uint32_t id;
    bool existent;
    bool req_impl;

    bool enable_signal_support;

    Device_params* dparams;
    Device_impl* dimpl;

    Device_create_state_func* create_state;
    bool (*set_audio_rate)(const struct Device*, Device_states*, int32_t);
    bool (*set_buffer_size)(const struct Device*, Device_states*, int32_t);
    void (*update_tempo)(const struct Device*, Device_states*, double);
    void (*reset)(const struct Device*, Device_states*);
    Device_process_signal_func* process_signal;

    Device_set_control_var_generic_func* set_control_var_generic;
    Device_slide_control_var_float_target_func* slide_control_var_float_target;
    Device_slide_control_var_float_length_func* slide_control_var_float_length;
    Device_osc_speed_cv_float_func* osc_speed_cv_float;
    Device_osc_depth_cv_float_func* osc_depth_cv_float;
    Device_osc_speed_slide_cv_float_func* osc_speed_slide_cv_float;
    Device_osc_depth_slide_cv_float_func* osc_depth_slide_cv_float;

    Device_init_control_vars_func* init_control_vars;
    Device_init_control_var_float_func* init_control_var_float;

    bool existence[DEVICE_PORT_TYPES][KQT_DEVICE_PORTS_MAX];
};


/**
 * Initialise the Device.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param req_impl   \c true if the Device requires a Device implementation,
 *                   otherwise \c false.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_init(Device* device, bool req_impl);


/**
 * Return the ID of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   The ID of the Device.
 */
uint32_t Device_get_id(const Device* device);


/**
 * Find out if the Device has a complete type.
 *
 * All audio units have a complete type; a processor has a complete type if it
 * has a Device implementation.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if the Device has a complete type, otherwise \c false.
 */
bool Device_has_complete_type(const Device* device);


/**
 * Set the existent status of the Device.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param existent   The existent flag.
 */
void Device_set_existent(Device* device, bool existent);


/**
 * Get the existent status of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if the Device is existent, otherwise \c false.
 */
bool Device_is_existent(const Device* device);


/**
 * Set a Device implementation of the Device.
 *
 * A previously set Device implementation will be destroyed.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param dimpl    The Device implementation, or \c NULL.
 *
 * \return   \c true if successful, or \c false if initialisation of Device
 *           implementation failed.
 */
bool Device_set_impl(Device* device, Device_impl* dimpl);


/**
 * Create a new Device state for the Device.
 *
 * \param device        The Device -- must not be \c NULL.
 * \param audio_rate    The audio rate -- must be > \c 0.
 * \param buffer_size   The audio buffer size -- must be >= \c 0.
 *
 * \return   The new Device state if successful, or \c NULL if memory
 *           allocation failed.
 */
Device_state* Device_create_state(
        const Device* device, int32_t audio_rate, int32_t buffer_size);


/**
 * Set a state creator for the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param creator   The creator function, or \c NULL for default creator.
 */
void Device_set_state_creator(
        Device* device, Device_state* (*creator)(const Device*, int32_t, int32_t));


/**
 * Register the audio rate set function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param set      The audio rate set function -- must not be \c NULL.
 */
void Device_register_set_audio_rate(
        Device* device, bool (*set)(const Device*, Device_states*, int32_t));


/**
 * Register the audio buffer size set function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param set      The audio buffer size set function -- must not be \c NULL.
 */
void Device_register_set_buffer_size(
        Device* device, bool (*set)(const Device*, Device_states*, int32_t));


/**
 * Register the tempo update function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param update   The tempo update function -- must not be \c NULL.
 */
void Device_register_update_tempo(
        Device* device, void (*update)(const Device*, Device_states*, double));


/**
 * Set the playback reset function of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param reset    The reset function -- must not be \c NULL.
 */
void Device_set_reset(Device* device, void (*reset)(const Device*, Device_states*));


/**
 * Set mixed signal processing support.
 *
 * Note that mixed signals may be always disabled for certain Devices.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param enabled   \c true if mixed signals should be enabled, otherwise
 *                  \c false.
 */
void Device_set_mixed_signals(Device* device, bool enabled);


/**
 * Get mixed signal support information.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if \a device has mixed signals enabled, otherwise \c false.
 */
bool Device_get_mixed_signals(const Device* device);


/**
 * Set the signal process function of the Device.
 *
 * \param device           The Device -- must not be \c NULL.
 * \param process_signal   The signal process function, or \c NULL.
 */
void Device_set_process(Device* device, Device_process_signal_func* process_signal);


/**
 * Set function for setting control variables.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param set_func   The generic Value setting function -- must not be \c NULL.
 */
void Device_register_set_control_var_generic(
        Device* device, Device_set_control_var_generic_func* set_func);


/**
 * Set functions for sliding floating-point control variables.
 *
 * \param device              The Device -- must not be \c NULL.
 * \param slide_target_func   The slide target function -- must not be \c NULL.
 * \param slide_length_func   The slide length function -- must not be \c NULL.
 */
void Device_register_slide_control_var_float(
        Device* device,
        Device_slide_control_var_float_target_func* slide_target_func,
        Device_slide_control_var_float_length_func* slide_length_func);


/**
 * Set functions for oscillating floating-point control variables.
 *
 * \param device             The Device -- must not be \c NULL.
 * \param speed_func         The oscillation depth function -- must not be \c NULL.
 * \param depth_func         The oscillation depth function -- must not be \c NULL.
 * \param speed_slide_func   The oscillation depth slide function -- must
 *                           not be \c NULL.
 * \param depth_slide_func   The oscillation depth slide function -- must
 *                           not be \c NULL.
 */
void Device_register_osc_cv_float(
        Device* device,
        Device_osc_speed_cv_float_func* speed_func,
        Device_osc_depth_cv_float_func* depth_func,
        Device_osc_speed_slide_cv_float_func* speed_slide_func,
        Device_osc_depth_slide_cv_float_func* depth_slide_func);


/**
 * Set function for initialising control variables.
 *
 * \param device      The Device -- must not be \c NULL.
 * \param init_func   The initialisation function -- must not be \c NULL.
 */
void Device_register_init_control_vars(
        Device* device, Device_init_control_vars_func* init_func);


/**
 * Set function for initialising a single float control variable.
 *
 * \param device      The Device -- must not be \c NULL.
 * \param init_func   The initialisation function -- must not be \c NULL.
 */
void Device_register_init_control_var_float(
        Device* device, Device_init_control_var_float_func* init_func);


/**
 * Set existence of a port in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX.
 * \param exists   \c true if the port exists, otherwise \c false.
 */
void Device_set_port_existence(
        Device* device, Device_port_type type, int port, bool exists);


/**
 * Get existence of a port in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param type     The type of the port -- must be a valid type.
 * \param port     The port number -- must be >= \c 0 and
 *                 < \c KQT_DEVICE_PORTS_MAX.
 *
 * \return   \c true if the port exists, otherwise \c false.
 */
bool Device_get_port_existence(const Device* device, Device_port_type type, int port);


/**
 * Set the audio rate of Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param rate      The mixing rate -- must be > \c 0.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_set_audio_rate(const Device* device, Device_states* dstates, int32_t rate);


/**
 * Update the tempo of the Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param tempo     The new tempo -- must be finite and > \c 0.
 */
void Device_update_tempo(const Device* device, Device_states* dstates, double tempo);


/**
 * Resize the buffers of Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param size      The new buffer size -- must be > \c 0 and <=
 *                  \c KQT_BUFFER_SIZE_MAX.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_set_buffer_size(const Device* device, Device_states* dstates, int32_t size);


/**
 * Reset the internal playback state of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 */
void Device_reset(const Device* device, Device_states* dstates);


/**
 * Synchronise the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if memory allocation failed.
 */
bool Device_sync(Device* device);


/**
 * Synchronise the Device states.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 */
bool Device_sync_states(const Device* device, Device_states* dstates);


/**
 * Set a key in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param key      The key that changed -- must not be \c NULL.
 * \param sr       The Streader of the data -- must not be \c NULL.
 *
 * \return   \c true if successful, or \c false if a fatal error occurred.
 */
bool Device_set_key(Device* device, const char* key, Streader* sr);


/**
 * Notify a Device state of a Device key change.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device state collection -- must not be \c NULL.
 * \param key       The key -- must be valid.
 *
 * \return   \c true if successful, or \c false if a fatal error occurred.
 */
bool Device_set_state_key(const Device* device, Device_states* dstates, const char* key);


/**
 * Process audio in the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param states   The Device states -- must not be \c NULL.
 * \param wbs      The Work buffers -- must not be \c NULL.
 * \param start    The first frame to be processed -- must be less than the
 *                 buffer size.
 * \param until    The first frame not to be processed -- must be less than or
 *                 equal to the buffer size. If \a until <= \a start, nothing
 *                 will be cleared.
 * \param freq     The mixing frequency -- must be > \c 0.
 * \param tempo    The tempo -- must be > \c 0 and finite.
 */
void Device_process(
        const Device* device,
        Device_states* states,
        const Work_buffers* wbs,
        uint32_t start,
        uint32_t until,
        uint32_t freq,
        double tempo);


/**
 * Set new value of a Device control variable.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mode       The Device control variable mode.
 * \param random     The Random source -- must not be \c NULL.
 * \param channel    The Channel -- must not be \c NULL if
 *                   \a mode == \c DEVICE_CONTROL_VAR_MODE_VOICE.
 * \param var_name   The name of the control variable -- must not be \c NULL.
 * \param value      The new value -- must not be \c NULL and must have a type
 *                   of \c VALUE_TYPE_BOOL, \c VALUE_TYPE_INT,
 *                   \c VALUE_TYPE_FLOAT or \c VALUE_TYPE_TSTAMP.
 */
void Device_set_control_var_generic(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel,
        const char* var_name,
        const Value* value);


/**
 * Slide to a new value of a floating-point Device control variable.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mode       The Device control variable mode.
 * \param channel    The Channel -- must not be \c NULL.
 * \param var_name   The name of the control variable, or \c NULL.
 * \param value      The target value -- must be finite.
 */
void Device_slide_control_var_float_target(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        double value);


/**
 * Set slide length of a floating-point Device control variable.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mode       The Device control variable mode.
 * \param channel    The Channel -- must not be \c NULL.
 * \param var_name   The name of the control variable, or \c NULL.
 * \param length     The new slide length -- must not be \c NULL.
 */
void Device_slide_control_var_float_length(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Tstamp* length);


/**
 * Set oscillation speed of a floating-point Device control variable.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mode       The Device control variable mode.
 * \param channel    The Channel -- must not be \c NULL.
 * \param var_name   The name of the control variable, or \c NULL.
 * \param speed      The oscillation speed -- must be >= \c 0.
 */
void Device_osc_speed_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        double speed);


/**
 * Set oscillation depth of a floating-point Device control variable.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mode       The Device control variable mode.
 * \param channel    The Channel -- must not be \c NULL.
 * \param var_name   The name of the control variable, or \c NULL.
 * \param depth      The oscillation depth -- must be finite.
 */
void Device_osc_depth_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        double depth);


/**
 * Set oscillation speed slide of a floating-point Device control variable.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mode       The Device control variable mode.
 * \param channel    The Channel -- must not be \c NULL.
 * \param var_name   The name of the control variable, or \c NULL.
 * \param length     The slide length of the speed -- must not be \c NULL.
 */
void Device_osc_speed_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Tstamp* length);


/**
 * Set oscillation depth slide of a floating-point Device control variable.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mode       The Device control variable mode.
 * \param channel    The Channel -- must not be \c NULL.
 * \param var_name   The name of the control variable, or \c NULL.
 * \param length     The slide length of the depth -- must not be \c NULL.
 */
void Device_osc_depth_slide_cv_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Tstamp* length);


/**
 * Initialise all control variables of the Device.
 *
 * \param device    The Device -- must not be \c NULL.
 * \param dstates   The Device states -- must not be \c NULL.
 * \param mode      The Device control variable mode.
 * \param random    Global Random source -- must not be \c NULL if
 *                  \a mode == \c DEVICE_CONTROL_VAR_MODE_MIXED.
 * \param channel   The Channel -- must not be \c NULL if
 *                  \a mode == \c DEVICE_CONTROL_VAR_MODE_VOICE.
 */
void Device_init_control_vars(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Random* random,
        Channel* channel);


/**
 * Initialise a single float control variable of the Device.
 *
 * \param device     The Device -- must not be \c NULL.
 * \param dstates    The Device states -- must not be \c NULL.
 * \param mode       The Device control variable mode.
 * \param channel    The Channel -- must not be \c NULL.
 * \param var_name   The variable name -- must not be \c NULL.
 * \param controls   The Linear controls of the float variable -- must not be \c NULL.
 */
void Device_init_control_var_float(
        const Device* device,
        Device_states* dstates,
        Device_control_var_mode mode,
        Channel* channel,
        const char* var_name,
        const Linear_controls* controls);


/**
 * Print a textual description of the Device.
 *
 * \param device   The Device -- must not be \c NULL.
 * \param out      The output file -- must not be \c NULL.
 */
void Device_print(const Device* device, FILE* out);


/**
 * Deinitialise the Device.
 *
 * \param device   The Device, or \c NULL.
 */
void Device_deinit(Device* device);


#endif // K_DEVICE_H


