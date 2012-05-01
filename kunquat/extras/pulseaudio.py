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

from __future__ import print_function
import ctypes
import sys
import time
import wave

__all__ = ['Simple', 'Poll', 'PulseAudioError']


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


PA_OPERATION_RUNNING = 0
PA_OPERATION_DONE = 1
PA_OPERATION_CANCELLED = 2

PA_CONTEXT_NOFLAGS = 0
PA_CONTEXT_NOAUTOSPAWN = 1
PA_CONTEXT_NOFAIL = 2

PA_CONTEXT_UNCONNECTED = 0
PA_CONTEXT_CONNECTING = 1
PA_CONTEXT_AUTHORIZING = 2
PA_CONTEXT_SETTING_NAME = 3
PA_CONTEXT_READY = 4
PA_CONTEXT_FAILED = 5
PA_CONTEXT_TERMINATED = 6

PA_STREAM_UNCONNECTED = 0
PA_STREAM_CREATING = 1
PA_STREAM_READY = 2
PA_STREAM_FAILED = 3
PA_STREAM_TERMINATED = 4
PA_STREAM_ADJUST_LATENCY = 0x2000

PA_SEEK_RELATIVE = 0


class _BufferAttr(ctypes.Structure):
    _fields_ = [('maxlength', ctypes.c_uint32),
                ('tlength', ctypes.c_uint32),
                ('prebuf', ctypes.c_uint32),
                ('minreq', ctypes.c_uint32),
                ('fragsize', ctypes.c_uint32),
               ]


class Poll(object):

    """An interface for a poll-style PulseAudio connection.

    """

    def __init__(self,
                 client_name,
                 stream_name,
                 rate=48000,
                 channels=2,
                 file_out=False):
        """Create a new PulseAudio connection.

        Arguments:
        client_name -- A descriptive name for the client.
        stream_name -- A descriptive name for the stream.

        Optional arguments:
        rate        -- Mixing frequency in frames per second (default:
                       48000).
        channels    -- Number of output channels (default: 2).
        file_out    -- Additional output to _kqtest.wav.

        """
        self.rate = 48000
        self.channels = 2
        self._main = None
        self._api = None
        self._context = None
        self._stream = None
        self._main = _pulse.pa_mainloop_new()
        if not self._main:
            raise PulseAudioError('Could not create PulseAudio main loop')
        self._api = _pulse.pa_mainloop_get_api(self._main)

        self._context = _pulse.pa_context_new(self._api, client_name)
        if not self._context:
            self._cleanup()
            raise PulseAudioError('Could not create '
                                  'PulseAudio connection context')
        if _pulse.pa_context_connect(self._context, None, 0, None) < 0:
            self._cleanup()
            raise PulseAudioError('Could not connect to PulseAudio server')
        retries = 1000
        state = self.context_state()
        while state != PA_CONTEXT_READY:
            _pulse.pa_mainloop_iterate(self._main, 1, None)
            retries -= 1
            state = self.context_state()
            if state == PA_CONTEXT_FAILED or retries < 0:
                self._cleanup()
                raise PulseAudioError('Could not connect to '
                                      'PulseAudio server.')

        ss = _SampleSpec(SAMPLE_FLOAT32, rate, channels)
        self._stream = _pulse.pa_stream_new(self._context,
                                            stream_name,
                                            ctypes.byref(ss),
                                            None)
        if not self._stream:
            self._cleanup()
            raise PulseAudioError('Could not create PulseAudio stream')
        buffering = _BufferAttr(ctypes.c_uint32(-1),
                                _pulse.pa_usec_to_bytes(200000,
                                                        ctypes.byref(ss)),
                                ctypes.c_uint32(-1),
                                ctypes.c_uint32(-1),
                                ctypes.c_uint32(-1))
        if _pulse.pa_stream_connect_playback(self._stream,
                                             None,
                                             ctypes.byref(buffering),
                                             PA_STREAM_ADJUST_LATENCY,
                                             None,
                                             None) < 0:
            self._cleanup()
            raise PulseAudioError('Could not connect PulseAudio stream')

        self._file_out = None
        if file_out:
            self._file_out = wave.open('_kqtest.wav', 'wb')
            self._file_out.setsampwidth(2)
            self._file_out.setframerate(rate)
            self._file_out.setnchannels(channels)
        self._last_write = None

    def context_state(self):
        return _pulse.pa_context_get_state(self._context)

    def error(self):
        return _pulse.pa_strerror(_pulse.pa_context_errno(self._context))

    def iterate(self):
        _pulse.pa_mainloop_iterate(self._main, 0, None)

    def ready(self):
        return self.context_state() == PA_CONTEXT_READY and \
               self.stream_state() == PA_STREAM_READY

    def stream_state(self):
        return _pulse.pa_stream_get_state(self._stream)

    def try_write(self, *data):
        if len(data) != self.channels:
            raise ValueError('Wrong number of output channel buffers')
        frame_count = len(data[0])
        if not self.ready() or frame_count > self.writable_size():
            self.iterate()
            return False
        #print(time.time() - self._last_write if self._last_write else '')
        #self._last_write = time.time()
        self.write(*data)
        self.iterate()
        return True

    def writable_size(self):
        bytes_per_frame = 4 * self.channels
        return _pulse.pa_stream_writable_size(self._stream) // bytes_per_frame

    def write(self, *data):
        if len(data) != self.channels:
            raise ValueError('Wrong number of output channel buffers')
        frame_count = len(data[0])
        if frame_count > self.writable_size():
            raise ValueError('The buffers are too large for the present '
                             'state of the stream')
        cdata = (ctypes.c_float * (frame_count * self.channels))()
        for channel in xrange(self.channels):
            if len(data[channel]) != frame_count:
                raise ValueError('Output channel buffer lengths do not match')
            cdata[channel::self.channels] = data[channel]
        bytes_per_frame = 4 * self.channels
        if _pulse.pa_stream_write(self._stream,
                                  cdata,
                                  bytes_per_frame * frame_count,
                                  None,
                                  0,
                                  PA_SEEK_RELATIVE) < 0:
            raise PulseAudioError(_pulse.pa_strerror(
                                  _pulse.pa_context_errno(self._context)))
        if self._file_out:
            odata = [0] * (frame_count * self.channels)
            for channel in xrange(self.channels):
                odata[channel::self.channels] = data[channel]
            for i, e in enumerate(odata):
                odata[i] = wave.struct.pack('h', int(e * 32767))
            ocdata = ''.join(odata)
            self._file_out.writeframes(ocdata)

    def _cleanup(self):
        if self._stream:
            _pulse.pa_stream_disconnect(self._stream)
            _pulse.pa_stream_unref(self._stream)
        if self._context:
            _pulse.pa_context_disconnect(self._context)
            _pulse.pa_context_unref(self._context)
        self._stream = None
        self._context = None
        self._api = None
        if self._main:
            _pulse.pa_mainloop_quit(self._main)
            _pulse.pa_mainloop_free(self._main)
        self._main = None

    def __del__(self):
        self._cleanup()
        if self._file_out:
            self._file_out.close()


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


_pulse = ctypes.CDLL('libpulse.so')

_pulse.pa_strerror.argtypes = [ctypes.c_int]
_pulse.pa_strerror.restype = ctypes.c_char_p

_pulse.pa_mainloop_new.argtypes = []
_pulse.pa_mainloop_new.restype = ctypes.c_void_p
_pulse.pa_mainloop_free.argtypes = [ctypes.c_void_p]
_pulse.pa_mainloop_get_api.argtypes = [ctypes.c_void_p]
_pulse.pa_mainloop_get_api.restype = ctypes.c_void_p
_pulse.pa_mainloop_iterate.argtypes = [ctypes.c_void_p,
                                       ctypes.c_int,
                                       ctypes.POINTER(ctypes.c_int)]
_pulse.pa_mainloop_iterate.restype = ctypes.c_int

_pulse.pa_proplist_new.argtypes = []
_pulse.pa_proplist_new.restype = ctypes.c_void_p
_pulse.pa_proplist_free.argtypes = [ctypes.c_void_p]
_pulse.pa_proplist_sets.argtypes = [ctypes.c_void_p,
                                    ctypes.c_char_p,
                                    ctypes.c_char_p]
_pulse.pa_proplist_sets.restype = ctypes.c_int

_pulse.pa_operation_unref.argtypes = [ctypes.c_void_p]
_pulse.pa_operation_cancel.argtypes = [ctypes.c_void_p]
_pulse.pa_operation_get_state.argtypes = [ctypes.c_void_p]
_pulse.pa_operation_get_state.restype = ctypes.c_int

_pulse.pa_context_new.argtypes = [ctypes.c_void_p,
                                  ctypes.c_char_p]
_pulse.pa_context_new.restype = ctypes.c_void_p
_pulse.pa_context_new_with_proplist.argtypes = [ctypes.c_void_p,
                                                ctypes.c_char_p,
                                                ctypes.c_void_p]
_pulse.pa_context_new_with_proplist.restype = ctypes.c_void_p
_pulse.pa_context_unref.argtypes = [ctypes.c_void_p]
_pulse.pa_context_connect.argtypes = [ctypes.c_void_p,
                                      ctypes.c_char_p,
                                      ctypes.c_int,
                                      ctypes.c_void_p]
_pulse.pa_context_connect.restype = ctypes.c_int
_pulse.pa_context_disconnect.argtypes = [ctypes.c_void_p]
_pulse.pa_context_get_state.argtypes = [ctypes.c_void_p]
_pulse.pa_context_get_state.restype = ctypes.c_int
_pulse.pa_context_errno.argtypes = [ctypes.c_void_p]
_pulse.pa_context_errno.restype = ctypes.c_int

_pulse.pa_stream_new.argtypes = [ctypes.c_void_p,
                                 ctypes.c_char_p,
                                 ctypes.POINTER(_SampleSpec),
                                 ctypes.c_void_p]
_pulse.pa_stream_new.restype = ctypes.c_void_p
_pulse.pa_stream_new_with_proplist.argtypes = [ctypes.c_void_p,
                                               ctypes.c_char_p,
                                               ctypes.POINTER(_SampleSpec),
                                               ctypes.c_void_p,
                                               ctypes.c_void_p]
_pulse.pa_stream_new_with_proplist.restype = ctypes.c_void_p
_pulse.pa_stream_unref.argtypes = [ctypes.c_void_p]
_pulse.pa_stream_get_state.argtypes = [ctypes.c_void_p]
_pulse.pa_stream_get_state.restype = ctypes.c_int
_pulse.pa_stream_connect_playback.argtypes = [ctypes.c_void_p,
                                              ctypes.c_char_p, # sink
                                              ctypes.c_void_p, # buff. attrs
                                              ctypes.c_int, # add. flags
                                              ctypes.c_void_p, # volume
                                              ctypes.c_void_p] # sync stream
_pulse.pa_stream_connect_playback.restype = ctypes.c_int
_pulse.pa_stream_disconnect.argtypes = [ctypes.c_void_p]
_pulse.pa_stream_disconnect.restype = ctypes.c_int
_pulse.pa_stream_write.argtypes = [ctypes.c_void_p,
                                   ctypes.c_void_p,
                                   ctypes.c_size_t,
                                   ctypes.c_void_p, # free_cb
                                   ctypes.c_int64, # offset
                                   ctypes.c_int] # seek mode
_pulse.pa_stream_write.restype = ctypes.c_int
_pulse.pa_stream_writable_size.argtypes = [ctypes.c_void_p]
_pulse.pa_stream_writable_size.restype = ctypes.c_size_t

_pulse.pa_usec_to_bytes.argtypes = [ctypes.c_uint64, # pa_usec_t t
                                    ctypes.POINTER(_SampleSpec)]
_pulse.pa_usec_to_bytes.restype = ctypes.c_size_t


