

/*
 * Author: Tomi Jylhä-Ollila, Finland 2010-2019
 *
 * This file is part of Kunquat.
 *
 * CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
 *
 * To the extent possible under law, Kunquat Affirmers have waived all
 * copyright and related or neighboring rights to Kunquat.
 */


#include <init/devices/Device.h>

#include <containers/Bit_array.h>
#include <debug/assert.h>
#include <decl.h>
#include <init/Background_loader.h>
#include <init/devices/Device_impl.h>
#include <mathnum/common.h>
#include <string/common.h>
#include <Value.h>

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>


bool Device_init(Device* device, bool req_impl)
{
    rassert(device != NULL);

    static uint32_t id = 1;
    device->id = id;
    ++id;

    device->existent = false;
    device->req_impl = req_impl;

    device->enable_signal_support = false;

    device->dparams = NULL;
    device->dimpl = NULL;

    device->create_state = new_Device_state_plain;

    for (int port_type = 0; port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        device->existence[port_type] = NULL;
        device->last_existence_set[port_type] = 0;
    }

    for (int port_type = 0; port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        device->existence[port_type] = new_Bit_array(KQT_DEVICE_PORTS_MAX);
        if (device->existence[port_type] == NULL)
        {
            Device_deinit(device);
            return false;
        }
    }

    device->dparams = new_Device_params();
    if (device->dparams == NULL)
    {
        Device_deinit(device);
        return false;
    }

    return true;
}


uint32_t Device_get_id(const Device* device)
{
    rassert(device != NULL);
    return device->id;
}


bool Device_has_complete_type(const Device* device)
{
    rassert(device != NULL);
    return !device->req_impl || (device->dimpl != NULL);
}


void Device_set_existent(Device* device, bool existent)
{
    rassert(device != NULL);
    device->existent = existent;
    return;
}


bool Device_is_existent(const Device* device)
{
    rassert(device != NULL);
    return device->existent;
}


void Device_set_impl(Device* device, Device_impl* dimpl, Background_loader* bkg_loader)
{
    rassert(device != NULL);
    rassert(bkg_loader != NULL);

    if (device->dimpl != NULL)
    {
        // Background loader tasks may be accessing the old device->dimpl,
        // so let's make sure we don't interfere with that
        if (!Background_loader_add_delayed_task(
                    bkg_loader,
                    (Background_loader_delayed_callback*)del_Device_impl,
                    device->dimpl))
        {
            Background_loader_wait_idle(bkg_loader);
            del_Device_impl(device->dimpl);
        }
        device->dimpl = NULL;
    }

    if (dimpl != NULL)
        Device_impl_set_device(dimpl, device);
    device->dimpl = dimpl;

    return;
}


const Device_impl* Device_get_impl(const Device* device)
{
    rassert(device != NULL);
    return device->dimpl;
}


Device_state* Device_create_state(
        const Device* device, int32_t audio_rate, int32_t buffer_size)
{
    rassert(device != NULL);
    rassert(device->create_state != NULL);
    rassert(audio_rate > 0);
    rassert(buffer_size >= 0);

    return device->create_state(device, audio_rate, buffer_size);
}


void Device_set_state_creator(
        Device* device, Device_state* (*creator)(const Device*, int32_t, int32_t))
{
    rassert(device != NULL);

    if (creator != NULL)
        device->create_state = creator;
    else
        device->create_state = new_Device_state_plain;

    return;
}


void Device_set_mixed_signals(Device* device, bool enabled)
{
    rassert(device != NULL);
    device->enable_signal_support = enabled;
    return;
}


bool Device_get_mixed_signals(const Device* device)
{
    rassert(device != NULL);
    return device->enable_signal_support;
}


void Device_set_port_existence(
        Device* device, Device_port_type type, int port, bool exists)
{
    rassert(device != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);
    rassert(device->existence[type] != NULL);

    Bit_array_set(device->existence[type], port, exists);
    if (exists)
        device->last_existence_set[type] = max(device->last_existence_set[type], port);

    return;
}


bool Device_get_port_existence(const Device* device, Device_port_type type, int port)
{
    rassert(device != NULL);
    rassert(type < DEVICE_PORT_TYPES);
    rassert(port >= 0);
    rassert(port < KQT_DEVICE_PORTS_MAX);
    rassert(device->existence[type] != NULL);

    return Bit_array_get(device->existence[type], port);
}


bool Device_validate_ports(const Device* device, Error* error)
{
    rassert(device != NULL);
    rassert(error != NULL);

    static const char* port_type_names[] =
    {
        "receive",
        "send",
        NULL,
    };

    for (int port_type = 0; port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        const int last_existence_set = device->last_existence_set[port_type];
        int port = 0;
        for (; port <= last_existence_set; ++port)
        {
            if (!Device_get_port_existence(device, port_type, port))
                break;
        }
        const int gap_start = port;
        for (++port; port <= last_existence_set; ++port)
        {
            if (Device_get_port_existence(device, port_type, port))
            {
                Error_set(
                        error,
                        ERROR_FORMAT,
                        "Port gap detected at %s port %d followed by"
                            " an existing port at %d",
                        port_type_names[port_type],
                        gap_start,
                        port);
                return false;
            }
        }
    }

    return true;
}


bool Device_sync(Device* device, Background_loader* bkg_loader)
{
    rassert(device != NULL);
    rassert(bkg_loader != NULL);

    // Set existing keys on dimpl
    if (device->dimpl != NULL)
    {
        Device_params_iter* iter = Device_params_iter_init(
                DEVICE_PARAMS_ITER_AUTO, device->dparams);

        const char* key = Device_params_iter_get_next_key(iter);
        while (key != NULL)
        {
            if (!Device_impl_set_key(device->dimpl, key, bkg_loader))
                return false;

            key = Device_params_iter_get_next_key(iter);
        }
    }

    return true;
}


bool Device_sync_states(const Device* device, Device_states* dstates)
{
    rassert(device != NULL);
    rassert(dstates != NULL);

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(
                dstates, Device_get_id(device));

        Device_params_iter* iter = Device_params_iter_init(
                DEVICE_PARAMS_ITER_AUTO, device->dparams);

        const char* key = Device_params_iter_get_next_key(iter);
        while (key != NULL)
        {
            if (!Device_impl_set_state_key(device->dimpl, dstate, key))
                return false;

            key = Device_params_iter_get_next_key(iter);
        }
    }

    return true;
}


bool Device_set_key(
        Device* device,
        const char* key,
        int version,
        Streader* sr,
        Background_loader* bkg_loader)
{
    rassert(device != NULL);
    rassert(key != NULL);
    rassert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));
    rassert(version >= 0);
    rassert(sr != NULL);
    rassert(bkg_loader != NULL);

    if (Streader_is_error_set(sr))
        return false;

    if (!Device_params_parse_value(device->dparams, key, version, sr, bkg_loader))
        return false;

    if ((device->dimpl != NULL) &&
            !Device_impl_set_key(device->dimpl, key + 2, bkg_loader))
    {
        Streader_set_memory_error(
                sr, "Could not allocate memory for device key %s", key);
        return false;
    }

    return true;
}


bool Device_set_state_key(
        const Device* device,
        Device_states* dstates,
        const char* key)
{
    rassert(device != NULL);
    rassert(dstates != NULL);
    rassert(key != NULL);
    rassert(string_has_prefix(key, "i/") || string_has_prefix(key, "c/"));

    if (device->dimpl != NULL)
    {
        Device_state* dstate = Device_states_get_state(
                dstates, Device_get_id(device));
        return Device_impl_set_state_key(device->dimpl, dstate, key + 2);
    }

    return true;
}


void Device_print(const Device* device, FILE* out)
{
    rassert(device != NULL);
    rassert(out != NULL);

    bool printed = false;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (!Device_get_port_existence(device, DEVICE_PORT_TYPE_SEND, port))
            continue;

        if (!printed)
        {
            fprintf(out, "Registered send ports:\n");
            printed = true;
        }

        fprintf(out, "  Port %02x\n", port);
    }

    printed = false;
    for (int port = 0; port < KQT_DEVICE_PORTS_MAX; ++port)
    {
        if (!Device_get_port_existence(device, DEVICE_PORT_TYPE_RECV, port))
            continue;

        if (!printed)
        {
            fprintf(out, "Registered receive ports:\n");
            printed = true;
        }

        fprintf(out, "  Port %02x\n", port);
    }

    return;
}


void Device_deinit(Device* device)
{
    if (device == NULL)
        return;

    del_Device_impl(device->dimpl);
    device->dimpl = NULL;

    del_Device_params(device->dparams);
    device->dparams = NULL;

    for (int port_type = 0; port_type < DEVICE_PORT_TYPES; ++port_type)
    {
        del_Bit_array(device->existence[port_type]);
        device->existence[port_type] = NULL;
    }

    return;
}


