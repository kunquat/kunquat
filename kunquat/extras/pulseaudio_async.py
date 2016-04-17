# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

"""A wrapper for the asynchronous PulseAudio library.

This module provides limited support for the asynchronous PulseAudio
interface. Designed for Kunquat music tools, the module only exists
because there is no widespread Python wrapper yet. As soon as one
becomes available in major distributions, this module will be removed.

"""


import ctypes
import queue

from .pulseaudio_def import *

__all__ = ['Async', 'PulseAudioError']


class Async():

    """Asynchronous access to PulseAudio.

    """

    def __init__(self,
            client_name,
            stream_name,
            audio_cb,
            rate=48000,
            latency=0.02):
        """Create a new PulseAudio connection object.

        Arguments:
        client_name -- A descriptive name for the client.
        stream_name -- A descriptive name for the stream.
        audio_cb    -- Callback function to call when more audio is
                       needed.  The callback must accept the
                       following arguments:
                       nframes -- Number of frames to be rendered.
                       The function must return the audio data in a
                       tuple/list where each element contains the audio
                       data of one channel.

        Optional arguments:
        rate        -- Audio rate in frames per second (default: 48000).
        latency     -- Desired latency in seconds (default: 0.02).  The
                       actual latency may differ.

        """
        assert type(client_name) == str
        assert type(stream_name) == str
        assert audio_cb
        assert rate >= 1
        assert rate <= 192000
        assert latency > 0

        self._client_name = client_name
        self._stream_name = stream_name
        self._audio_cb = audio_cb
        self._rate = int(rate)
        self._channels = 2
        self._latency = int(latency * 1000000)

        self._ml = None
        self._context = None
        self._stream = None
        self._context_q = queue.Queue()
        self._stream_q = queue.Queue()

        # Create C callback objects
        def cs_cb(context, userdata):
            self._context_state_cb(context, userdata)
        self._cs_cb = _pa_context_notify_cb(cs_cb)

        def ss_cb(stream, userdata):
            self._stream_state_cb(stream, userdata)
        self._ss_cb = _pa_stream_notify_cb(ss_cb)

        def sw_cb(stream, length, userdata):
            self._stream_write_cb(stream, length, userdata)
        self._sw_cb = _pa_stream_request_cb(sw_cb)

        def ssucc_cb(stream, success, userdata):
            self._stream_success_cb(stream, success, userdata)
        self._ssucc_cb = _pa_stream_success_cb(ssucc_cb)

    def init(self):
        """Init PulseAudio connection.

        Exceptions:
        PulseAudioError -- Raised on any PulseAudio failure.

        """
        # Create and start the threaded main loop
        self._ml = _pa.pa_threaded_mainloop_new()
        if not self._ml:
            raise PulseAudioError('Could not create main loop')
        if _pa.pa_threaded_mainloop_start(self._ml) < 0:
            raise PulseAudioError('Could not start main loop')

        # Get mainloop api
        api = _pa.pa_threaded_mainloop_get_api(self._ml)
        if not api:
            raise PulseAudioError(
                    'Could not retrieve abstraction layer vtable')

        # Create context
        self._context = _pa.pa_context_new(
                api, bytes(self._client_name, encoding='utf-8'))
        if not self._context:
            raise PulseAudioError('Could not create context')

        # Start connecting to server
        _pa.pa_threaded_mainloop_lock(self._ml)

        # Set context state callback
        _pa.pa_context_set_state_callback(self._context, self._cs_cb, None)

        # Connect
        if _pa.pa_context_connect(
                self._context, None, PA_CONTEXT_NOFLAGS, None) < 0:
            raise PulseAudioError('Could not connect to server')
        _pa.pa_threaded_mainloop_unlock(self._ml)

        # Wait for connection
        state = self._context_q.get(True, 2)
        while state != PA_CONTEXT_READY:
            if state == PA_CONTEXT_FAILED:
                raise PulseAudioError('Connection failed: {}'.format(
                    _pa.pa_strerror(_pa.pa_context_errno(self._context))
                    ))
            state = self._context_q.get(True, 2)

        # Create stream
        assert not self._stream
        ss = SampleSpec(PA_SAMPLE_FLOAT32, self._rate, self._channels)
        self._stream = _pa.pa_stream_new(
                self._context,
                bytes(self._stream_name, encoding='utf-8'),
                ss, None)
        if not self._stream:
            raise PulseAudioError('Could not create stream')

        # Set stream state callback
        _pa.pa_stream_set_state_callback(self._stream, self._ss_cb, None)

        # Set write callback
        _pa.pa_stream_set_write_callback(self._stream, self._sw_cb, None)

        # Connect stream
        buf_attr = BufferAttr(
                ctypes.c_uint32(-1), # maxlength
                _pa.pa_usec_to_bytes(self._latency, ctypes.byref(ss)),
                ctypes.c_uint32(-1), # prebuf
                ctypes.c_uint32(-1), # minreq
                ctypes.c_uint32(-1), # fragsize
                )

        if _pa.pa_stream_connect_playback(
                self._stream,
                None, # default device
                ctypes.byref(buf_attr),
                PA_STREAM_START_CORKED | PA_STREAM_ADJUST_LATENCY,
                None, # default volume
                None # no sync stream
                ) < 0:
            raise PulseAudioError('Could not connect stream')

        # Wait until stream is ready
        state = self._stream_q.get(True, 2)
        while state != PA_STREAM_READY:
            if state == PA_STREAM_FAILED:
                raise PulseAudioError('Stream creation failed: {}'.format(
                    _pa.pa_strerror(_pa.pa_context_errno(self._context))
                    ))
            state = self._stream_q.get(True, 2)

    def _context_state_cb(self, context, userdata):
        assert context == self._context

        state = _pa.pa_context_get_state(self._context)
        self._context_q.put(state)

    def _stream_state_cb(self, stream, userdata):
        assert (stream == self._stream) or not self._stream

        state = _pa.pa_stream_get_state(stream)
        self._stream_q.put(state)
        if state == PA_STREAM_FAILED:
            print('Stream failed: {}'.format(
                _pa.pa_strerror(_pa.pa_context_errno(self._context))
                ))

    def _stream_write_cb(self, stream, length, userdata):
        assert stream
        assert (stream == self._stream) or not self._stream

        if not self._stream:
            return

        bytes_per_frame = 4 * self._channels
        frame_count = length // bytes_per_frame

        # Create buffer
        cdata = (ctypes.c_float * (frame_count * self._channels))()

        # Make sure the stream is not corked
        # This fixes a PulseAudio annoyance after init
        if not _pa.pa_stream_is_corked(self._stream):
            # Get audio data
            bufs = self._audio_cb(frame_count)
            assert len(bufs) == self._channels

            # Fill buffer with audio data
            for ch in range(self._channels):
                received_frames = len(bufs[ch])
                if received_frames != frame_count:
                    raise PulseAudioError(
                            'Expected {} frames, received {}'.format(
                            frame_count, received_frames)
                        )
                buf = bufs[ch]
                buf.extend([0] * (frame_count - len(buf)))
                cdata[ch::self._channels] = buf

        # Write data
        if _pa.pa_stream_write(
                self._stream,
                cdata,
                bytes_per_frame * frame_count,
                None,
                0,
                PA_SEEK_RELATIVE) < 0:
            print('Could not write audio data: {}'.format(
                _pa.pa_strerror(_pa.pa_context_errno(self._context))
                ))

    def _stream_success_cb(self, stream, success, userdata):
        assert stream
        assert (stream == self._stream) or not self._stream

    def play(self):
        """Start playback."""
        assert self._stream
        _pa.pa_threaded_mainloop_lock(self._ml)
        assert _pa.pa_stream_get_state(self._stream) == PA_STREAM_READY
        op = _pa.pa_stream_cork(self._stream, 0, self._ssucc_cb, None)
        assert op
        _pa.pa_operation_unref(op)
        _pa.pa_threaded_mainloop_unlock(self._ml)

    def stop(self):
        """Stop playback."""
        assert self._stream
        _pa.pa_threaded_mainloop_lock(self._ml)
        op = _pa.pa_stream_cork(self._stream, 1, self._ssucc_cb, None)
        assert op
        _pa.pa_operation_unref(op)
        _pa.pa_threaded_mainloop_unlock(self._ml)

    def deinit(self):
        """Closes a PulseAudio connection.

        The connection may be opened again with init().

        """
        # Destroy stream
        if self._stream:
            assert self._ml
            _pa.pa_threaded_mainloop_lock(self._ml)
            _pa.pa_stream_disconnect(self._stream)
            _pa.pa_stream_unref(self._stream)
            self._stream = None
            _pa.pa_threaded_mainloop_unlock(self._ml)

        # Destroy context
        if self._context:
            assert self._ml
            _pa.pa_threaded_mainloop_lock(self._ml)
            _pa.pa_context_disconnect(self._context)
            _pa.pa_context_unref(self._context)
            self._context = None
            _pa.pa_threaded_mainloop_unlock(self._ml)

        # Destroy main loop
        if self._ml:
            _pa.pa_threaded_mainloop_stop(self._ml)
            _pa.pa_threaded_mainloop_free(self._ml)
            self._ml = None

    def __del__(self):
        self.deinit()


_pa = ctypes.CDLL('libpulse.so')

_pa_ml = ctypes.c_void_p
_pa_ml_api = ctypes.c_void_p
_pa_context = ctypes.c_void_p
_pa_stream = ctypes.c_void_p
_pa_cvolume = ctypes.c_void_p
_pa_usec = ctypes.c_uint64
_pa_operation = ctypes.c_void_p

_pa_context_notify_cb = ctypes.CFUNCTYPE(
        None,
        _pa_context,
        ctypes.c_void_p) # userdata

_pa_stream_notify_cb = ctypes.CFUNCTYPE(
        None,
        _pa_stream,
        ctypes.c_void_p) # userdata

_pa_stream_request_cb = ctypes.CFUNCTYPE(
        None,
        _pa_stream,
        ctypes.c_size_t,
        ctypes.c_void_p) # userdata

_pa_stream_success_cb = ctypes.CFUNCTYPE(
        None,
        _pa_stream,
        ctypes.c_int, # success
        ctypes.c_void_p) # userdata


# Error stuff

_pa.pa_strerror.argtypes = [ctypes.c_int]
_pa.pa_strerror.restype = ctypes.c_char_p


# Threaded mainloop & api

_pa.pa_threaded_mainloop_new.argtypes = []
_pa.pa_threaded_mainloop_new.restype = _pa_ml

_pa.pa_threaded_mainloop_start.argtypes = [_pa_ml]
_pa.pa_threaded_mainloop_start.restype = ctypes.c_int

_pa.pa_threaded_mainloop_lock.argtypes = [_pa_ml]
_pa.pa_threaded_mainloop_lock.restype = None
_pa.pa_threaded_mainloop_unlock.argtypes = [_pa_ml]
_pa.pa_threaded_mainloop_unlock.restype = None

_pa.pa_threaded_mainloop_get_api.argtypes = [_pa_ml]
_pa.pa_threaded_mainloop_get_api.restype = _pa_ml_api

_pa.pa_threaded_mainloop_stop.argtypes = [_pa_ml]
_pa.pa_threaded_mainloop_stop.restype = None

_pa.pa_threaded_mainloop_free.argtypes = [_pa_ml]
_pa.pa_threaded_mainloop_free.restype = None


# Context

_pa.pa_context_new.argtypes = [_pa_ml_api, ctypes.c_char_p]
_pa.pa_context_new.restype = _pa_context

_pa.pa_context_errno.argtypes = [_pa_context]
_pa.pa_context_errno.restype = ctypes.c_int

_pa.pa_context_set_state_callback.argtypes = [
        _pa_context,
        _pa_context_notify_cb,
        ctypes.c_void_p] # userdata
_pa.pa_context_set_state_callback.restype = None

_pa.pa_context_get_state.argtypes = [_pa_context]
_pa.pa_context_get_state.restype = ctypes.c_int # pa_context_state

_pa.pa_context_connect.argtypes = [
        _pa_context,
        ctypes.c_char_p,
        ctypes.c_int, # pa_context_flags_t
        ctypes.c_void_p] # pa_spawn_api
_pa.pa_context_connect.restype = ctypes.c_int

_pa.pa_context_disconnect.argtypes = [_pa_context]
_pa.pa_context_disconnect.restype = None

_pa.pa_context_unref.argtypes = [_pa_context]
_pa.pa_context_unref.restype = None


# Stream

_pa.pa_stream_new.argtypes = [
        _pa_context,
        ctypes.c_char_p,
        ctypes.POINTER(SampleSpec),
        ctypes.c_void_p] # channel map
_pa.pa_stream_new.restype = _pa_stream

_pa.pa_stream_set_state_callback.argtypes = [
        _pa_stream,
        _pa_stream_notify_cb,
        ctypes.c_void_p]
_pa.pa_stream_set_state_callback.restype = None

_pa.pa_stream_get_state.argtypes = [_pa_stream]
_pa.pa_stream_get_state.restype = ctypes.c_int

_pa.pa_stream_set_write_callback.argtypes = [
        _pa_stream,
        _pa_stream_request_cb,
        ctypes.c_void_p]
_pa.pa_stream_set_write_callback.restype = None

_pa.pa_stream_connect_playback.argtypes = [
        _pa_stream,
        ctypes.c_char_p,
        ctypes.POINTER(BufferAttr),
        ctypes.c_int, # pa_stream_flags
        _pa_cvolume,
        _pa_stream] # sync stream
_pa.pa_stream_connect_playback.restype = ctypes.c_int

_pa.pa_stream_begin_write.argtypes = [
        _pa_stream,
        ctypes.POINTER(ctypes.c_void_p),
        ctypes.POINTER(ctypes.c_size_t)]
_pa.pa_stream_begin_write.restype = ctypes.c_int

_pa.pa_stream_write.argtypes = [
        _pa_stream,
        ctypes.c_void_p, # data
        ctypes.c_size_t, # nbytes
        ctypes.c_void_p, # free_cb
        ctypes.c_int64, # offset
        ctypes.c_int] # pa_seek_mode
_pa.pa_stream_write.restype = ctypes.c_int

_pa.pa_stream_cork.argtypes = [
        _pa_stream,
        ctypes.c_int, # 1 == pause, 0 == resume
        _pa_stream_success_cb,
        ctypes.c_void_p] # userdata
_pa.pa_stream_cork.restype = _pa_operation

_pa.pa_stream_is_corked.argtypes = [_pa_stream]
_pa.pa_stream_is_corked.restype = ctypes.c_int

_pa.pa_stream_disconnect.argtypes = [_pa_stream]
_pa.pa_stream_disconnect.restype = ctypes.c_int

_pa.pa_stream_unref.argtypes = [_pa_stream]
_pa.pa_stream_unref.restype = None


# Operations

_pa.pa_operation_get_state.argtypes = [_pa_operation]
_pa.pa_operation_get_state.restype = ctypes.c_int # pa_operation_state

_pa.pa_operation_unref.argtypes = [_pa_operation]
_pa.pa_operation_unref.restype = None


# Helper functions

_pa.pa_usec_to_bytes.argtypes = [_pa_usec, ctypes.POINTER(SampleSpec)]
_pa.pa_usec_to_bytes.restype = ctypes.c_size_t

_pa.pa_bytes_to_usec.argtypes = [ctypes.c_uint64, ctypes.POINTER(SampleSpec)]
_pa.pa_bytes_to_usec.restype = _pa_usec


