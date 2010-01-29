# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

"""A wrapper for the PulseAudio library.

This module provides limited support for the simple PulseAudio
interface. Designed for Kunquat music tools, the module only exists
because there is no widespread Python wrapper yet. As soon as one
becomes available in major distributions, this module will be removed.

"""

import ctypes
import sys

__all__ = ['Simple', 'PulseAudioError']


if sys.byteorder == 'little':
    SAMPLE_FLOAT32 = 5
else:
    SAMPLE_FLOAT32 = 6


STREAM_PLAYBACK = 1
STREAM_RECORD = 2


class _SampleSpec(ctypes.Structure):
    _fields_ = [('format', ctypes.c_int),
                ('rate', ctypes.c_uint32),
                ('channels', ctypes.c_uint8)]


class Simple(object):

    """An interface for a simple PulseAudio connection.

    Public methods:
    write -- Write audio data.
    drain -- Wait until all data written is actually played.

    """

    def __init__(self,
                 client_name,
                 stream_name,
                 rate=48000,
                 channels=2,
                 format=SAMPLE_FLOAT32,
                 server_name=None,
                 direction=STREAM_PLAYBACK,
                 sink_name=None,
                 channel_map=None,
                 buffer_attr=None):
        """Create a new PulseAudio connection.

        Arguments:
        client_name -- A descriptive name for the client.
        stream_name -- A descriptive name for the stream.

        Optional arguments:
        rate        -- Mixing frequency in frames per second (default:
                       48000).
        channels    -- Number of output channels (default: 2).
        format      -- Frame format (default: SAMPLE_FLOAT32).
        server_name -- Name of the server.
        direction   -- Whether the stream is a playback (default) or
                       recording stream.
        sink_name   -- Sink name.
        channel_map -- The channel map (not yet supported).
        buffer_attr -- Buffering attributes (not yet supported).

        Exceptions:
        PulseAudioError -- Raised if the PulseAudio call fails.

        """
        ss = _SampleSpec(format, rate, channels)
        error = ctypes.c_int(0)
        self._connection = _simple.pa_simple_new(server_name,
                                                 client_name,
                                                 direction,
                                                 sink_name,
                                                 stream_name,
                                                 ctypes.byref(ss),
                                                 None,
                                                 None,
                                                 ctypes.byref(error))
        if not self._connection:
            raise PulseAudioError(_simple.pa_strerror(error))
        self.rate = rate
        self.channels = channels

    def write(self, *data):
        """Write audio data to the output stream.

        Arguments:
        data -- The output buffers, each channel as a separate
                parameter.

        Exceptions:
        PulseAudioError -- Raised if the PulseAudio call fails.
        ValueError      -- Wrong number of output buffers, or the
                           buffer lengths do not match.

        """
        if len(data) != self.channels:
            raise ValueError('Wrong number of output channel buffers')
        frame_count = len(data[0])
        cdata = (ctypes.c_float * (frame_count * self.channels))()
        for channel in xrange(self.channels):
            if len(data[channel]) != frame_count:
                raise ValueError('Output channel buffer lengths do not match')
            cdata[channel::self.channels] = data[channel]
        bytes_per_frame = 4 * self.channels
        error = ctypes.c_int(0)
        if _simple.pa_simple_write(self._connection,
                                   cdata,
                                   bytes_per_frame * frame_count,
                                   ctypes.byref(error)) < 0:
            raise PulseAudioError(_simple.pa_strerror(error))

    def drain(self):
        """Wait until all data written is actually played.

        Exceptions:
        PulseAudioError -- Raised if the PulseAudio call fails.

        """
        error = ctypes.c_int(0)
        if _simple.pa_simple_drain(self._connection, ctypes.byref(error)) < 0:
            raise PulseAudioError(_simple.pa_strerror(error))

    def __del__(self):
        if self._connection:
            _simple.pa_simple_free(self._connection)
        self._connection = None


class PulseAudioError(Exception):
    """Class for PulseAudio-related errors.

    This error is raised whenever an underlying PulseAudio function
    call fails.

    """


_simple = ctypes.CDLL('libpulse-simple.so')

_simple.pa_strerror.argtypes = [ctypes.c_int]
_simple.pa_strerror.restype = ctypes.c_char_p

_simple.pa_simple_new.argtypes = [ctypes.c_char_p,
                                  ctypes.c_char_p,
                                  ctypes.c_int, # pa_stream_direction_t
                                  ctypes.c_char_p,
                                  ctypes.c_char_p,
                                  ctypes.POINTER(_SampleSpec),
                                  ctypes.c_void_p, # channel map
                                  ctypes.c_void_p, # buffering attrs,
                                  ctypes.POINTER(ctypes.c_int)]
_simple.pa_simple_new.restype = ctypes.c_void_p

_simple.pa_simple_free.argtypes = [ctypes.c_void_p]

_simple.pa_simple_write.argtypes = [ctypes.c_void_p,
                                    ctypes.c_void_p,
                                    ctypes.c_size_t,
                                    ctypes.POINTER(ctypes.c_int)]
_simple.pa_simple_write.restype = ctypes.c_int

_simple.pa_simple_drain.argtypes = [ctypes.c_void_p,
                                    ctypes.POINTER(ctypes.c_int)]
_simple.pa_simple_drain.restype = ctypes.c_int


