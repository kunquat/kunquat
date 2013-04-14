# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import ctypes
import sys


class PulseAudioError(Exception):
    """Class for PulseAudio-related errors.

    This error is raised whenever an underlying PulseAudio function
    call fails.

    """


def _make_c_enum(names):
    for i, name in enumerate(names):
        globals()[name] = i


# Sample format

_pa_sample_format = [
        'PA_SAMPLE_U8',
        'PA_SAMPLE_ALAW',
        'PA_SAMPLE_ULAW',
        'PA_SAMPLE_S16LE',
        'PA_SAMPLE_S16BE',
        'PA_SAMPLE_FLOAT32LE',
        'PA_SAMPLE_FLOAT32BE',
        'PA_SAMPLE_S32LE',
        'PA_SAMPLE_S32BE',
        'PA_SAMPLE_S24LE',
        'PA_SAMPLE_S24BE',
        'PA_SAMPLE_S24_32LE',
        'PA_SAMPLE_S24_32BE',
        'PA_SAMPLE_MAX',
    ]
_make_c_enum(_pa_sample_format)
PA_SAMPLE_INVALID = -1

if sys.byteorder == 'little':
    PA_SAMPLE_FLOAT32NE = PA_SAMPLE_FLOAT32LE
else:
    PA_SAMPLE_FLOAT32NE = PA_SAMPLE_FLOAT32BE
PA_SAMPLE_FLOAT32 = PA_SAMPLE_FLOAT32NE


# Context flags

PA_CONTEXT_NOFLAGS     = 0x0000
PA_CONTEXT_NOAUTOSPAWN = 0x0001
PA_CONTEXT_NOFAIL      = 0x0002

# Context state

_pa_context_state = [
        'PA_CONTEXT_UNCONNECTED',
        'PA_CONTEXT_CONNECTING',
        'PA_CONTEXT_AUTHORIZING',
        'PA_CONTEXT_SETTING_NAME',
        'PA_CONTEXT_READY',
        'PA_CONTEXT_FAILED',
        'PA_CONTEXT_TERMINATED',
    ]
_make_c_enum(_pa_context_state)


# Stream flags

PA_STREAM_NOFLAGS            = 0x0000
PA_STREAM_START_CORKED       = 0x0001
PA_STREAM_INTERPOLATE_TIMING = 0x0002
PA_STREAM_NOT_MONOTONIC      = 0x0004
PA_STREAM_AUTO_TIMING_UPDATE = 0x0008
PA_STREAM_NO_REMAP_CHANNELS  = 0x0010
PA_STREAM_NO_REMIX_CHANNELS  = 0x0020
PA_STREAM_FIX_FORMAT         = 0x0040
PA_STREAM_FIX_RATE           = 0x0080
PA_STREAM_FIX_CHANNELS       = 0x0100
PA_STREAM_DONT_MOVE          = 0x0200
PA_STREAM_VARIABLE_RATE      = 0x0400
PA_STREAM_PEAK_DETECT        = 0x0800
PA_STREAM_START_MUTED        = 0x1000
PA_STREAM_ADJUST_LATENCY     = 0x2000
PA_STREAM_EARLY_REQUESTS     = 0x4000
PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND = 0x8000
PA_STREAM_START_UNMUTED      = 0x10000
PA_STREAM_FAIL_ON_SUSPEND    = 0x20000
PA_STREAM_RELATIVE_VOLUME    = 0x40000
PA_STREAM_PASSTHROUGH        = 0x80000

# Stream state

_pa_stream_state = [
        'PA_STREAM_UNCONNECTED',
        'PA_STREAM_CREATING',
        'PA_STREAM_READY',
        'PA_STREAM_FAILED',
        'PA_STREAM_TERMINATED',
    ]
_make_c_enum(_pa_stream_state)

# Stream direction

_pa_stream_direction = [
        'PA_STREAM_NODIRECTION',
        'PA_STREAM_PLAYBACK',
        'PA_STREAM_RECORD',
        'PA_STREAM_UPLOAD',
    ]
_make_c_enum(_pa_stream_direction)

# Seek mode

# No idea why PulseAudio specifies the values explicitly instead of
# relying on the enum construct; following the same style here
PA_SEEK_RELATIVE         = 0
PA_SEEK_ABSOLUTE         = 1
PA_SEEK_RELATIVE_ON_READ = 2
PA_SEEK_RELATIVE_END     = 3


# Operation state

_pa_operation_state = [
        'PA_OPERATION_RUNNING',
        'PA_OPERATION_DONE',
        'PA_OPERATION_CANCELLED',
    ]
_make_c_enum(_pa_operation_state)


class SampleSpec(ctypes.Structure):
    _fields_ = [
            ('format', ctypes.c_int), # _pa_sample_format
            ('rate', ctypes.c_uint32),
            ('channels', ctypes.c_uint8),
        ]

class BufferAttr(ctypes.Structure):
    _fields_ = [
            ('maxlength', ctypes.c_uint32),
            ('tlength', ctypes.c_uint32),
            ('prebuf', ctypes.c_uint32),
            ('minreq', ctypes.c_uint32),
            ('fragsize', ctypes.c_uint32),
        ]

"""
PA_CHANNELS_MAX = 32

PA_VOLUME_NORM = 0x10000
PA_VOLUME_MUTED = 0
PA_VOLUME_MAX = 2**31 - 1

class _CVolume(ctypes.Structure):
    _fields_ = [
            ('channels', ctypes.c_uint8),
            ('values', ctypes.c_uint32 * PA_CHANNELS_MAX), # pa_volume
        ]
"""


