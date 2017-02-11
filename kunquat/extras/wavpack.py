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

"""A wrapper for accessing WavPack sound data.

"""

import ctypes
from io import BytesIO, SEEK_SET, SEEK_CUR, SEEK_END


_EOF = -1


class _WavPackBase():

    def __init__(self):
        self._wpc = None

    def close(self):
        if self._wpc != None:
            _wavpack.WavpackCloseFile(self._wpc)
            self._wpc = None


class _WavPackRBase(_WavPackBase):

    def __init__(self, file_obj, convert_to_float):
        super().__init__()
        self._f = file_obj
        self._convert_to_float = convert_to_float

        self._channels = 0
        self._length = 0
        self._is_float = False
        self._bits = 0
        self._rate = 0

        self._reader = None
        self._bytes_per_sample = 0
        self._push_back = None

        self._f.seek(0, SEEK_END)
        self._file_length = self._f.tell()
        self._f.seek(0, SEEK_SET)

    def get_channels(self):
        return self._channels

    def get_length(self):
        return self._length

    def is_float(self):
        return self._is_float or self._convert_to_float

    def get_bits(self):
        return self._bits

    def get_audio_rate(self):
        return self._rate

    def read(self, frame_count=float('inf')):
        '''Read audio data.

        Optional arguments:
        frame_count -- Maximum number of frames to be read.  If this
                       argument is omitted, all the data will be read.

        Return value:
        A tuple containing audio data for each channel.  Buffers
        shorter than frame_count frames indicate that the end has
        been reached.

        '''
        frames_left = frame_count
        if frames_left == float('inf'):
            frame_count = 65536

        chunk = [[] for _ in range(self._channels)]

        cdata = (ctypes.c_int32 * (frame_count * self._channels))()

        # Read data
        while frames_left > 0:
            actual_frame_count = _wavpack.WavpackUnpackSamples(
                    self._wpc, cdata, frame_count)
            if self._is_float:
                fdata = ctypes.cast(cdata, ctypes.POINTER(ctypes.c_float))
                # For some reason, getting a 0-length slice of fdata doesn't work
                # as expected, so we need to guard against that
                if actual_frame_count > 0:
                    for ch in range(self._channels):
                        ch_data = fdata[
                                ch:actual_frame_count * self._channels:self._channels]
                        chunk[ch].extend(ch_data)
            else:
                for ch in range(self._channels):
                    ch_data = cdata[ch:actual_frame_count * self._channels:self._channels]
                    chunk[ch].extend(ch_data)

            if actual_frame_count < frame_count:
                break

            frames_left -= actual_frame_count

        # Scale integer data
        if not self._is_float:
            if self._convert_to_float:
                scale = 1 / 2**(self._bits - 1)
                for ch in range(self._channels):
                    chunk[ch] = [s * scale for s in chunk[ch]]
            else:
                shift = 32 - self._bits
                for ch in range(self._channels):
                    chunk[ch] = [s << shift for s in chunk[ch]]

        return tuple(chunk)

    def close(self):
        super().close()
        if self._f:
            self._f.close()
            self._f = None

    def _prepare(self):
        def read_bytes_cb(id_, data, bcount):
            return self._read_bytes(data, bcount)
        self._read_bytes_cb = _read_bytes(read_bytes_cb)

        def get_pos_cb(id_):
            return self._get_pos()
        self._get_pos_cb = _get_pos(get_pos_cb)

        def set_pos_abs_cb(id_, pos):
            return self._set_pos_abs(pos)
        self._set_pos_abs_cb = _set_pos_abs(set_pos_abs_cb)

        def set_pos_rel_cb(id_, delta, mode):
            return self._set_pos_rel(delta, mode)
        self._set_pos_rel_cb = _set_pos_rel(set_pos_rel_cb)

        def push_back_byte_cb(id_, c):
            return self._push_back_byte(c)
        self._push_back_byte_cb = _push_back_byte(push_back_byte_cb)

        def get_length_cb(id_):
            return self._get_length()
        self._get_length_cb = _get_length(get_length_cb)

        def can_seek_cb(id_):
            return self._can_seek()
        self._can_seek_cb = _can_seek(can_seek_cb)

        def write_bytes_cb(id_, data, bcount):
            return self._write_bytes(data, bcount)
        self._write_bytes_cb = _write_bytes(write_bytes_cb)

        self._reader = _WavpackStreamReader(
            self._read_bytes_cb,
            self._get_pos_cb,
            self._set_pos_abs_cb,
            self._set_pos_rel_cb,
            self._push_back_byte_cb,
            self._get_length_cb,
            self._can_seek_cb,
            self._write_bytes_cb)

        error_msg = (ctypes.c_char * 81)()
        flags = _OPEN_2CH_MAX | _OPEN_NORMALIZE

        self._wpc = _wavpack.WavpackOpenFileInputEx(
                self._reader, None, None, error_msg, flags, 0)
        if not self._wpc:
            error_str = str(error_msg, encoding='utf-8')
            raise WavPackError(
                    'Error while setting up WavPack reader: {}'.format(error_str))

        mode = _wavpack.WavpackGetMode(self._wpc)

        self._channels = _wavpack.WavpackGetReducedChannels(self._wpc)
        self._length = _wavpack.WavpackGetNumSamples(self._wpc)
        self._is_float = (mode & _MODE_FLOAT) != 0
        self._bits = _wavpack.WavpackGetBitsPerSample(self._wpc)
        self._bytes_per_sample = _wavpack.WavpackGetBytesPerSample(self._wpc)
        self._rate = _wavpack.WavpackGetSampleRate(self._wpc)

    def _read_bytes(self, data, bcount):
        read_data = bytearray(bcount)

        read_start = 0
        read_count = bcount
        if self._push_back != None:
            read_data[0] = self._push_back
            self._push_back = None
            read_start = 1
            read_count -= 1
            self._f.seek(1, SEEK_CUR)

        fbytes = self._f.read(read_count)
        available_count = read_start + len(fbytes)
        assert available_count <= bcount
        read_data[read_start:] = fbytes

        for i in range(available_count):
            data[i] = read_data[i]

        return available_count

    def _get_pos(self):
        pos = self._f.tell()
        return pos

    def _set_pos_abs(self, pos):
        self._f.seek(pos)
        self._push_back = None
        return 0

    def _set_pos_rel(self, delta, mode):
        self._f.seek(delta, mode)
        self._push_back = None
        return 0

    def _push_back_byte(self, c):
        if self._push_back != None:
            return _EOF
        if self._get_pos() == 0:
            return _EOF

        self._f.seek(-1, SEEK_CUR)
        self._push_back = c

        return c

    def _get_length(self):
        return self._file_length

    def _can_seek(self):
        return 1

    def _write_bytes(self, data, bcount):
        assert False


class WavPackR(_WavPackRBase):

    def __init__(self, fname, convert_to_float=True):
        '''Create a new readable WavPack audio file.

        Arguments:
        fname -- Input file name.

        Optional arguemnts:
        convert_to_float -- Convert audio data to float.  If set to
                            False, integer audio data will be scaled
                            to 32-bit integers.

        '''
        super().__init__(open(fname, 'rb'), convert_to_float)
        self._prepare()

    def __del__(self):
        self.close()


class WavPackRMem(_WavPackRBase):

    def __init__(self, data, convert_to_float=True):
        '''Create a new readable WavPack stream from data in memory.

        Arguments:
        data -- Input data.

        Optional arguments:
        convert_to_float -- Convert audio data to float.  If set to
                            False, integer audio data will be scaled
                            to 32-bit integers.

        '''
        super().__init__(BytesIO(data), convert_to_float)
        self._prepare()

    def __del__(self):
        self.close()


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

        flags = _CONFIG_VERY_HIGH_FLAG
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
            raise ValueError('Expected {} output channel{}, got {}'.format(
                self._channels, '' if self._channels == 1 else 's', len(data)))

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

        if self._f:
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
        ctypes.POINTER(ctypes.c_char), # data
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
        ctypes.POINTER(ctypes.c_char), # data
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


