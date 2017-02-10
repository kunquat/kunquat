# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

'''A wrapper for accessing libsamplerate.

'''

import ctypes
import math


SRC_SINC_BEST_QUALITY   = 0
SRC_SINC_MEDIUM_QUALITY = 1
SRC_SINC_FASTEST        = 2
SRC_ZERO_ORDER_HOLD     = 3
SRC_LINEAR              = 4


class SampleRate():

    _BUF_FRAME_COUNT = 4096

    def __init__(self, converter_type, channels):
        '''Crate a new sample rate converter.

        Arguments:
        converter_type -- The converter type.
        channels       -- Number of channels.

        '''
        self._channels = channels

        error = ctypes.c_int(0)
        self._state = _samplerate.src_new(converter_type, channels, ctypes.byref(error))
        if self._state == None:
            err_cstr = _samplerate.src_strerror(error)
            raise SampleRateError('Could not set up sample rate conversion: {}'.format(
                str(err_cstr, encoding='utf-8')))

        self._input_data = [[] for _ in range(self._channels)]
        self._allow_input = True

        self._in_buf = (ctypes.c_float * (self._BUF_FRAME_COUNT * self._channels))()
        self._out_buf = (ctypes.c_float * (self._BUF_FRAME_COUNT * self._channels))()

        self._data = _SRC_DATA(
                ctypes.cast(self._in_buf, ctypes.POINTER(ctypes.c_float)),
                ctypes.cast(self._out_buf, ctypes.POINTER(ctypes.c_float)),
                0, 0, # input & output frames
                0, 0, # filled by libsamplerate
                0, # end of input
                1.0) # ratio

        self._prev_orig_frame_count = 0
        self._prev_est_frame_count = 0
        self._cur_orig_frame_count = 0
        self._cur_est_frame_count = 0
        self._out_frame_count = 0

    def set_ratio(self, ratio, smooth=True):
        '''Set sample rate conversion ratio.

        Arguments:
        ratio  -- The conversion ratio.
        smooth -- Use smooth transition of conversion ratio.

        '''
        if math.isnan(ratio) or math.isinf(ratio) or (ratio <= 0.0):
            raise ValueError('Invalid sample rate conversion ratio: {}'.format(ratio))

        if self._data.src_ratio != ratio:
            self._prev_orig_frame_count += self._cur_orig_frame_count
            self._prev_est_frame_count += self._cur_est_frame_count
            self._cur_orig_frame_count = 0
            self._cur_est_frame_count = 0

        self._data.src_ratio = ratio

        if not smooth:
            error = _samplerate.src_set_ratio(self._state, ratio)
            if error != 0:
                err_cstr = _samplerate.src_strerror(error)
                raise SampleRateError('Could not set new conversion ratio: {}'.format(
                    str(err_cstr, encoding='utf-8')))

    def add_input_data(self, *data, end_of_input=True):
        '''Add input audio data to the sample rate converter.

        Arguments:
        data         -- The input data, each channel as a separate argument.
        end_of_input -- True if data is the end of input data.  It is an
                        error to call this function after a previous call
                        with this value set to True.

        '''
        if not self._allow_input:
            raise SampleRateError(
                    'More input data provided after indicating end of input')

        if len(data) != self._channels:
            raise ValueError('Expected {} input channels, got {}'.format(
                self._channels, len(data)))

        self._cur_orig_frame_count += len(data[0])
        self._cur_est_frame_count = int(
                self._cur_orig_frame_count * self._data.src_ratio)

        frame_count = len(data[0])
        for ch in range(self._channels):
            if len(data[ch]) != frame_count:
                raise ValueError('Input channel buffer lengths do not match')
            self._input_data[ch].extend(data[ch])

        if end_of_input:
            self._allow_input = False

    def _get_total_est_frame_count(self):
        return self._cur_est_frame_count + self._prev_est_frame_count

    def get_output_data(self):
        '''Get converted audio data.

        Return value:
        A tuple containing audio data for each channel.  The returned
        audio data contains all converted data of fed input data so
        far.

        '''

        frames_left = len(self._input_data[0])

        cur_offset = 0
        output_data = [[] for _ in range(self._channels)]

        def process():
            error = _samplerate.src_process(self._state, self._data)
            if error != 0:
                err_cstr = _samplerate.src_strerror(error)
                raise SampleRateError('Error while converting data: {}'.format(
                    str(err_cstr, encoding='utf-8')))

        def extend_output():
            generated = self._data.output_frames_gen
            self._out_frame_count += generated
            for ch in range(self._channels):
                output_data[ch].extend(self._data.data_out[
                    ch : ch + self._channels * generated : self._channels])

        while frames_left > 0:
            orig_frame_count = min(self._BUF_FRAME_COUNT, frames_left)
            last_chunk = (not self._allow_input) and (orig_frame_count == frames_left)

            # Fill input buffer
            orig_item_count = self._channels * orig_frame_count
            for ch in range(self._channels):
                self._in_buf[ch : ch + orig_item_count : self._channels] = (
                        self._input_data[ch][cur_offset:cur_offset + orig_frame_count])

            frame_count = orig_frame_count

            self._data.data_in = ctypes.cast(
                    self._in_buf, ctypes.POINTER(ctypes.c_float))
            self._data.input_frames = frame_count
            self._data.output_frames = self._BUF_FRAME_COUNT
            self._data.input_frames_used = -1
            self._data.output_frames_gen = 0
            self._data.end_of_input = 1 if last_chunk else 0

            # Convert
            while frame_count > 0:
                process()

                # Get converted data
                extend_output()

                assert self._data.input_frames_used >= 0

                frame_count -= self._data.input_frames_used

                # Move input pointer forwards in case we didn't consume all input
                elem_offset = self._channels * (orig_frame_count - frame_count)
                self._data.data_in = ctypes.cast(
                        ctypes.byref(
                            self._in_buf, elem_offset * ctypes.sizeof(ctypes.c_float)),
                        ctypes.POINTER(ctypes.c_float))

                self._data.input_frames = frame_count
                self._data.input_frames_used = 0
                self._data.output_frames_gen = 0

            cur_offset += orig_frame_count - frame_count
            frames_left -= orig_frame_count - frame_count

        if not self._allow_input:
            assert frames_left == 0

            # Make sure we get all the remaining output data
            self._in_buf[:] = [0.0] * (self._BUF_FRAME_COUNT * self._channels)
            self._data.input_frames = 0
            self._data.output_frames = self._BUF_FRAME_COUNT
            self._data.input_frames_used = 0
            self._data.output_frames_gen = 0
            self._data.end_of_input = 0

            process()

            while self._data.output_frames_gen > 0:
                extend_output()

                self._data.input_frames = 0
                self._data.output_frames = self._BUF_FRAME_COUNT
                self._data.input_frames_used = 0
                self._data.output_frames_gen = 0
                self._data.end_of_input = 1

                process()

            # Clear input
            self._input_data = [[] for _ in range(self._channels)]

        else:
            if frames_left == 0:
                self._input_data = [[] for _ in range(self._channels)]
            else:
                for ch in range(self._channels):
                    self._input_data[ch] = self._input_data[ch][-frames_left:]

        return tuple(output_data)

    def reset(self):
        '''Reset the internal state of the sample rate converter.

        '''
        error = _samplerate.src_reset(self._state)
        if error != 0:
            err_cstr = _samplerate.src_strerror(error)
            raise SampleRateError('Could not reset resampling state: {}'.format(
                str(err_cstr, encoding='utf-8')))

        self._input_data = [[] for _ in range(self._channels)]
        self._allow_input = True

    def __del__(self):
        if self._state == None:
            return

        _samplerate.src_delete(self._state)
        self._state = None


class SampleRateError(Exception):
    '''Class for libsamplerate-related errors.

    '''


_SRC_STATE = ctypes.c_void_p

class _SRC_DATA(ctypes.Structure):
    _fields_ = [('data_in', ctypes.POINTER(ctypes.c_float)),
                ('data_out', ctypes.POINTER(ctypes.c_float)),
                ('input_frames', ctypes.c_long),
                ('output_frames', ctypes.c_long),
                ('input_frames_used', ctypes.c_long),
                ('output_frames_gen', ctypes.c_long),
                ('end_of_input', ctypes.c_int),
                ('src_ratio', ctypes.c_double)]


_samplerate = ctypes.CDLL('libsamplerate.so')

_samplerate.src_strerror.argtypes = [ctypes.c_int]
_samplerate.src_strerror.restype = ctypes.c_char_p

_samplerate.src_new.argtypes = [
        ctypes.c_int, # converter type
        ctypes.c_int, # channels
        ctypes.POINTER(ctypes.c_int), # error code destination
    ]
_samplerate.src_new.restype = _SRC_STATE

_samplerate.src_process.argtypes = [_SRC_STATE, ctypes.POINTER(_SRC_DATA)]
_samplerate.src_process.restype = ctypes.c_int

_samplerate.src_reset.argtypes = [_SRC_STATE]
_samplerate.src_reset.restype = ctypes.c_int

_samplerate.src_set_ratio.argtypes = [_SRC_STATE, ctypes.c_double]
_samplerate.src_set_ratio.restype = ctypes.c_int

_samplerate.src_delete.argtypes = [_SRC_STATE]
_samplerate.src_delete.restype = _SRC_STATE


