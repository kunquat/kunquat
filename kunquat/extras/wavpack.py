# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

"""A wrapper for accessing WavPack sound data.

"""

import ctypes
from io import BytesIO


class _WavPackBase():

    def __init__(self):
        self._wpc = None

    def close(self):
        if self._wpc != None:
            _wavpack.WavpackCloseFile(self._wpc)
            self._wpc = None


class _WavPackWBase(_WavPackBase):

    def __init__(self, file_obj):
        super().__init__()
        self._f = file_obj

        self._channels = 0
        self._use_float = False
        self._bits = 0

        self._first_block = None

    def _prepare(self, rate, channels, use_float, bits):
        self._channels = channels
        self._use_float = use_float
        self._bits = bits

        if self._use_float and (bits != 32):
            raise ValueError('Bit depth must be 32 in floating-point mode')

        if self._bits in (8, 16, 32):
            bytes_per_sample = self._bits // 8
        elif bits == 24:
            bytes_per_sample = 4
        else:
            raise NotImplementedError('Bit depths not multiple by 8 are not supported')

        channel_mask = { 1: 4, 2: 3 }.get(self._channels, 0)

        def block_output_cb(id_, data, bcount):
            if not self._first_block:
                self._first_block = (ctypes.c_char * bcount)()
                self._first_block[:] = data[:bcount]
            return self._block_output(id_, data, bcount)
        self._block_output_cb = _WavpackBlockOutput(block_output_cb)

        self._wpc = _wavpack.WavpackOpenFileOutput(self._block_output_cb, None, None)
        if self._wpc == None:
            raise MemoryError('Could not allocate memory for WavPack context')

        flags = _CONFIG_VERY_HIGH_FLAG | _CONFIG_SKIP_WVX
        float_norm_exp = 0
        if self._use_float:
            flags |= _CONFIG_FLOAT_DATA
            float_norm_exp = 127

        config = _WavpackConfig(
                0,                  # bitrate
                0,                  # shaping_weight
                self._bits,         # bits_per_sample
                bytes_per_sample,   # bytes_per_sample
                0,                  # qmode
                flags,              # flags
                0,                  # xmode
                self._channels,     # num_channels
                float_norm_exp,     # float_norm_exp (127 for +/-1.0)
                0,                  # block_samples
                0,                  # extra_flags
                rate,               # sample_rate
                channel_mask,       # channel_mask
                (ctypes.c_ubyte * 16)(), # md5_checksum
                0,                  # md5_read
                0,                  # num_tag_strings
                None)               # tag_strings

        if not _wavpack.WavpackSetConfiguration(self._wpc, config, -1):
            raise WavPackError('Error while setting WavPack configuration: {}'.format(
                _wavpack.WavpackGetErrorMessage(self._wpc)))

        if not _wavpack.WavpackPackInit(self._wpc):
            raise WavPackError('Could not init WavPack packing: {}'.format(
                _wavpack.WavpackGetErrorMessage(self._wpc)))

    def _fill_buffer(self, frame_count, target, src):
        for ch in range(self._channels):
            if len(src[ch]) != frame_count:
                raise ValueError('Output channel buffer lengths do not match')
            target[ch::self._channels] = src[ch]

    def write(self, *data):
        """Write audio data.

        Arguments:
        data -- The output buffers, each channel as a separate argument.

        """
        assert self._wpc

        if len(data) != self._channels:
            raise ValueError('Expected {} output channels, got {}'.format(
                self._channels, len(data)))

        frame_count = len(data[0])
        if self._use_float:
            fdata = (ctypes.c_float * (frame_count * self._channels))()
            self._fill_buffer(frame_count, fdata, data)
            cdata = ctypes.cast(fdata, ctypes.POINTER(ctypes.c_int32))
        else:
            cdata = (ctypes.c_int32 * (frame_count * self._channels))()
            self._fill_buffer(frame_count, cdata, data)

        if not _wavpack.WavpackPackSamples(self._wpc, cdata, frame_count):
            raise WavPackError('Error while writing WavPack data: {}'.format(
                _wavpack.WavpackGetErrorMessage(self._wpc)))

    def close(self):
        super().close()
        if self._f:
            self._f.close()
            self._f = None

    def finalise(self):
        if self._wpc:
            if not _wavpack.WavpackFlushSamples(self._wpc):
                raise WavPackError('Error while flushing output: {}'.format(
                    _wavpack.WavpackGetErrorMessage(self._wpc)))

            if self._first_block:
                _wavpack.WavpackUpdateNumSamples(self._wpc, self._first_block)
                self._update_first_block(self._first_block)

    def _block_output(self, id_, data, bcount):
        if self._f:
            self._f.write(bytes(data[:bcount]))
        return 1

    def _update_first_block(self, block):
        if self._f:
            self._f.seek(0)
            self._f.write(bytes(block))


class WavPackW(_WavPackWBase):

    def __init__(
            self,
            fname,
            rate=48000,
            channels=1,
            use_float=False,
            bits=16):
        """Create a new writable WavPack file.

        Arguments:
        fname -- Output file name.

        Optional arguments:
        rate      -- Audio rate.
        channels  -- Number of output channels.
        use_float -- Use floating-point frames.
        bits      -- Bits per item.

        """
        super().__init__(open(fname, 'wb'))
        self._prepare(rate, channels, use_float, bits)

    def __del__(self):
        self.finalise()
        self.close()


class WavPackWMem(_WavPackWBase):

    def __init__(
            self,
            rate=48000,
            channels=2,
            use_float=False,
            bits=16):
        """Create a new writable WavPack stream to memory.

        Optional arguments:
        rate      -- Audio rate.
        channels  -- Number of output channels.
        use_float -- Use floating-point frames.
        bits      -- Bits per item.

        """
        super().__init__(BytesIO())

        self._data = None

        self._prepare(rate, channels, use_float, bits)

    def close(self):
        self.finalise()

        self._f.seek(0)
        self._data = self._f.read()

        super().close()

    def get_contents(self):
        self.close()
        return self._data

    def __del__(self):
        self.close()


class WavPackError(Exception):
    '''Class for WavPack-related errors.

    '''


_MODE_FLOAT     = 0x8

_CONFIG_FLOAT_DATA      = 0x80
_CONFIG_FAST_FLAG       = 0x200
_CONFIG_HIGH_FLAG       = 0x800
_CONFIG_VERY_HIGH_FLAG  = 0x1000
_CONFIG_SKIP_WVX        = 0x4000000

_OPEN_2CH_MAX   = 0x8
_OPEN_NORMALIZE = 0x10


_read_bytes = ctypes.CFUNCTYPE(
        ctypes.c_int32,
        ctypes.c_void_p, # id
        ctypes.c_void_p, # data
        ctypes.c_int32) # bcount

_get_pos = ctypes.CFUNCTYPE(
        ctypes.c_uint32,
        ctypes.c_void_p) # id

_set_pos_abs = ctypes.CFUNCTYPE(
        ctypes.c_int,
        ctypes.c_void_p, # id
        ctypes.c_uint32) # pos

_set_pos_rel = ctypes.CFUNCTYPE(
        ctypes.c_int,
        ctypes.c_void_p, # id
        ctypes.c_int32, # delta
        ctypes.c_int) # mode

_push_back_byte = ctypes.CFUNCTYPE(
        ctypes.c_int,
        ctypes.c_void_p, # id
        ctypes.c_int) # byte

_get_length = ctypes.CFUNCTYPE(
        ctypes.c_uint32,
        ctypes.c_void_p) # id

_can_seek = ctypes.CFUNCTYPE(
        ctypes.c_int,
        ctypes.c_void_p) # id

_write_bytes = ctypes.CFUNCTYPE(
        ctypes.c_int32,
        ctypes.c_void_p, # id
        ctypes.c_void_p, # data
        ctypes.c_int32) # bcount

class _WavpackStreamReader(ctypes.Structure):
    _fields_ = [
        ('read_bytes', _read_bytes),
        ('get_pos', _get_pos),
        ('set_pos_abs', _set_pos_abs),
        ('set_pos_rel', _set_pos_rel),
        ('push_back_byte', _push_back_byte),
        ('get_length', _get_length),
        ('can_seek', _can_seek),
        ('write_bytes', _write_bytes),
    ]


_WavpackContext = ctypes.c_void_p


class _WavpackConfig(ctypes.Structure):
    _fields_ = [
        ('bitrate', ctypes.c_float),
        ('shaping_weight', ctypes.c_float),
        ('bits_per_sample', ctypes.c_int),
        ('bytes_per_sample', ctypes.c_int),
        ('qmode', ctypes.c_int),
        ('flags', ctypes.c_int),
        ('xmode', ctypes.c_int),
        ('num_channels', ctypes.c_int),
        ('float_norm_exp', ctypes.c_int),
        ('block_samples', ctypes.c_int32),
        ('extra_flags', ctypes.c_int32),
        ('sample_rate', ctypes.c_int32),
        ('channel_mask', ctypes.c_int32),
        ('md5_checksum', ctypes.c_ubyte * 16),
        ('md5_read', ctypes.c_ubyte),
        ('num_tag_strings', ctypes.c_int),
        ('tag_strings', ctypes.POINTER(ctypes.c_char_p)),
    ]


_WavpackBlockOutput = ctypes.CFUNCTYPE(
        ctypes.c_int,
        ctypes.c_void_p, # id
        ctypes.POINTER(ctypes.c_char), # data
        ctypes.c_int32) # bcount


_wavpack = ctypes.CDLL('libwavpack.so')

_wavpack.WavpackGetErrorMessage.argtypes = [_WavpackContext]
_wavpack.WavpackGetErrorMessage.restype = ctypes.c_char_p

_wavpack.WavpackOpenFileInputEx.argtypes = [
        ctypes.POINTER(_WavpackStreamReader),
        ctypes.c_void_p, # wv_id
        ctypes.c_void_p, # wvc_id
        ctypes.c_char_p, # error
        ctypes.c_int, # flags
        ctypes.c_int] # norm_offset
_wavpack.WavpackOpenFileInputEx.restype = _WavpackContext

_wavpack.WavpackGetMode.argtypes = [_WavpackContext]
_wavpack.WavpackGetMode.restype = ctypes.c_int

_wavpack.WavpackGetReducedChannels.argtypes = [_WavpackContext]
_wavpack.WavpackGetReducedChannels.restype = ctypes.c_int

_wavpack.WavpackGetSampleRate.argtypes = [_WavpackContext]
_wavpack.WavpackGetSampleRate.restype = ctypes.c_uint32

_wavpack.WavpackGetBitsPerSample.argtypes = [_WavpackContext]
_wavpack.WavpackGetBitsPerSample.restype = ctypes.c_int

_wavpack.WavpackGetBytesPerSample.argtypes = [_WavpackContext]
_wavpack.WavpackGetBytesPerSample.restype = ctypes.c_int

_wavpack.WavpackGetNumSamples.argtypes = [_WavpackContext]
_wavpack.WavpackGetNumSamples.restype = ctypes.c_uint32

_wavpack.WavpackUnpackSamples.argtypes = [
        _WavpackContext, ctypes.POINTER(ctypes.c_int32), ctypes.c_uint32]
_wavpack.WavpackUnpackSamples.restype = ctypes.c_uint32

_wavpack.WavpackOpenFileOutput.argtypes = [
        _WavpackBlockOutput,
        ctypes.c_void_p, # wv_id
        ctypes.c_void_p] # wvc_id
_wavpack.WavpackOpenFileOutput.restype = _WavpackContext

_wavpack.WavpackSetConfiguration.argtypes = [
        _WavpackContext,
        ctypes.POINTER(_WavpackConfig),
        ctypes.c_uint32] # total_samples
_wavpack.WavpackSetConfiguration.restype = ctypes.c_int

_wavpack.WavpackPackInit.argtypes = [_WavpackContext]
_wavpack.WavpackPackInit.restype = ctypes.c_int

_wavpack.WavpackPackSamples.argtypes = [
        _WavpackContext,
        ctypes.POINTER(ctypes.c_int32), # sample_buffer
        ctypes.c_uint32] # sample_count
_wavpack.WavpackPackSamples.restype = ctypes.c_int

_wavpack.WavpackFlushSamples.argtypes = [_WavpackContext]
_wavpack.WavpackFlushSamples.restype = ctypes.c_int

_wavpack.WavpackUpdateNumSamples.argtypes = [
        _WavpackContext,
        ctypes.c_void_p] # first_block

_wavpack.WavpackCloseFile.argtypes = [_WavpackContext]
_wavpack.WavpackCloseFile.restype = _WavpackContext


