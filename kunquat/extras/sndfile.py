# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2012-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

"""A wrapper for accessing sound data through libsndfile.

"""

import ctypes
import ctypes.util
import os


FORMAT_WAV  = 0x010000
FORMAT_AU   = 0x020000
FORMAT_FLAC = 0x170000

FORMAT_PCM_S8 = 0x0001
FORMAT_PCM_16 = 0x0002
FORMAT_PCM_24 = 0x0003
FORMAT_PCM_32 = 0x0004

FORMAT_PCM_U8 = 0x0005

FORMAT_FLOAT  = 0x0006
FORMAT_DOUBLE = 0x0007

FORMAT_SUBMASK  = 0x0000ffff
FORMAT_TYPEMASK = 0x0fff0000
FORMAT_ENDMASK  = 0x30000000

SF_FALSE = 0
SF_TRUE = 1

SFM_READ = 0x10
SFM_WRITE = 0x20

SFC_SET_CLIPPING = 0x10c0

formats_map = {
        'wav':  FORMAT_WAV,
        'au':   FORMAT_AU,
        'flac': FORMAT_FLAC,
        }

bits_map = {
        8:  FORMAT_PCM_S8,
        16: FORMAT_PCM_16,
        24: FORMAT_PCM_24,
        32: FORMAT_PCM_32,
        }


class _SndFileBase():

    def __init__(self):
        self._channels = 0
        self._sf = None

        # Virtual I/O state
        self._pos = 0
        self._bytes = None

    def close(self):
        if self._sf != None:
            _sndfile.sf_close(self._sf)
            self._sf = None

    def _get_virtual_io_info(self):
        def get_filelen_cb(user_data):
            return self._get_filelen(user_data)
        self._get_filelen_cb = _sf_vio_get_filelen(get_filelen_cb)

        def seek_cb(offset, whence, user_data):
            return self._seek(offset, whence, user_data)
        self._seek_cb = _sf_vio_seek(seek_cb)

        def read_cb(ptr, count, user_data):
            return self._read(ptr, count, user_data)
        self._read_cb = _sf_vio_read(read_cb)

        def write_cb(ptr, count, user_data):
            return self._write(ptr, count, user_data)
        self._write_cb = _sf_vio_write(write_cb)

        def tell_cb(user_data):
            return self._tell(user_data)
        self._tell_cb = _sf_vio_tell(tell_cb)

        return _SF_VIRTUAL_IO(
                self._get_filelen_cb,
                self._seek_cb,
                self._read_cb,
                self._write_cb,
                self._tell_cb)

    def _get_filelen(self, user_data):
        return len(self._bytes)

    def _seek(self, offset, whence, user_data):
        if whence == os.SEEK_SET:
            self._pos = offset
        elif whence == os.SEEK_CUR:
            self._pos += offset
        elif whence == os.SEEK_END:
            self._pos = len(self._bytes) + offset
        return self._pos

    def _read(self, ptr, count, user_data):
        assert self._bytes != None
        actual_count = min(count, len(self._bytes) - self._pos)
        for i in range(actual_count):
            ptr[i] = self._bytes[self._pos + i]
        self._pos += actual_count
        return actual_count

    def _write(self, ptr, count, user_data):
        assert self._bytes != None
        source = ctypes.cast(ptr, ctypes.POINTER(ctypes.c_char))
        self._bytes[self._pos:self._pos + count] = source[:count]
        self._pos += count
        return count

    def _tell(self, user_data):
        return self._pos


class _SndFileRBase(_SndFileBase):

    def __init__(self, convert_to_float):
        super().__init__()
        self._convert_to_float = convert_to_float
        self._is_float = False
        self._bits = 0
        self._audio_rate = 0

    def set_format_info(self, sf_info):
        self._channels = sf_info.channels
        self._audio_rate = sf_info.samplerate

        sub_format = sf_info.format & FORMAT_SUBMASK
        if sub_format in (FORMAT_FLOAT, FORMAT_DOUBLE):
            self._is_float = True
            self._bits = 32
        else:
            self._is_float = False
            format_bit_map = {
                FORMAT_PCM_S8: 8,
                FORMAT_PCM_U8: 8,
                FORMAT_PCM_16: 16,
                FORMAT_PCM_24: 24,
                FORMAT_PCM_32: 32,
            }
            if sub_format in format_bit_map:
                self._bits = format_bit_map[sub_format]
            else:
                self._bits = 32
                self._convert_to_float = True

    def get_bits(self):
        return self._bits

    def is_float(self):
        return self._is_float or self._convert_to_float

    def get_channels(self):
        return self._channels

    def get_audio_rate(self):
        return self._audio_rate

    def read(self, frame_count=float('inf')):
        """Read audio data.

        Optional arguments:
        frame_count -- Maximum number of frames to be read.  If this
                       argument is omitted, all the data will be read.

        Return value:
        A tuple containing audio data for each channel.  Buffers
        shorter than frame_count frames indicate that the end has
        been reached.

        """
        frames_left = frame_count
        if frames_left == float('inf'):
            frame_count = 4096

        use_float = self._convert_to_float or self._is_float

        if use_float:
            cdata = (ctypes.c_float * (frame_count * self._channels))()
        else:
            cdata = (ctypes.c_int32 * (frame_count * self._channels))()
        chunk = [[] for _ in range(self._channels)]

        while frames_left > 0:
            if use_float:
                actual_frame_count = _sndfile.sf_readf_float(self._sf, cdata, frame_count)
            else:
                actual_frame_count = _sndfile.sf_readf_int(self._sf, cdata, frame_count)
            for ch in range(self._channels):
                channel_data = cdata[ch:actual_frame_count * self._channels:self._channels]
                chunk[ch].extend(channel_data)

            if actual_frame_count < frame_count:
                break

            frames_left -= actual_frame_count

        return tuple(chunk)


class SndFileR(_SndFileRBase):

    def __init__(self, fname, convert_to_float=True):
        """Create a new readable audio file.

        Arguments:
        fname -- Input file name.

        Optional arguments:
        convert_to_float -- Convert audio data to float.  If set to
                            False, integer audio data will be scaled
                            to 32-bit integers.

        """
        super().__init__(convert_to_float)

        info = _SF_INFO(0, 0, 0, 0, 0, 0)

        self._sf = _sndfile.sf_open(bytes(fname, encoding='utf-8'), SFM_READ, info)
        if not self._sf:
            err_cstr = _sndfile.sf_strerror(None)
            raise SndFileError(str(err_cstr, encoding='utf-8'))

        self.set_format_info(info)

    def __del__(self):
        self.close()


class SndFileRMem(_SndFileRBase):

    def __init__(self, data, convert_to_float=True):
        """Create a new readable audio stream from data in memory.

        Arguments:
        data -- Input data.

        Optional arguments:
        convert_to_float -- Convert audio data to float.  If set to
                            False, integer audio data will be scaled
                            to 32-bit integers.

        """
        super().__init__(convert_to_float)
        self._data = data

        self._bytes = bytes(self._data)

        vio = self._get_virtual_io_info()
        info = _SF_INFO(0, 0, 0, 0, 0, 0)

        self._sf = _sndfile.sf_open_virtual(vio, SFM_READ, info, None)
        if not self._sf:
            err_cstr = _sndfile.sf_strerror(None)
            raise SndFileError('Could not set up data access: {}'.format(
                str(err_cstr, encoding='utf-8')))

        self.set_format_info(info)

    def __del__(self):
        self.close()


class _SndFileWBase(_SndFileBase):

    def __init__(self):
        super().__init__()

    def _get_validated_sf_info(self, format, rate, channels, use_float, bits):
        fspec = formats_map[format]
        if use_float:
            fspec |= FORMAT_FLOAT
        else:
            if (format == 'wav') and (bits == 8):
                fspec |= FORMAT_PCM_U8
            else:
                fspec |= bits_map[bits]

        info = _SF_INFO(0, rate, channels, fspec, 0, 0)
        if not _sndfile.sf_format_check(info):
            num_mode = 'floating' if use_float else 'fixed'
            hdesc = '{}, {} point, {} bits'.format(format, num_mode, bits)
            raise ValueError('Unsupported format parameter combination:'
                    ' {}'.format(hdesc))

        return info

    def write(self, *data):
        """Write audio data.

        Arguments:
        data -- The output, either as a single buffer with interleaved
                channels, or each channel as a separate argument.

        """
        if len(data) == 1:
            buf = data[0]
            if len(buf) % self._channels != 0:
                raise ValueError('Data length {} is not divisible by expected'
                        ' number of channels {}'.format(len(data), self._channels))
            frame_count = len(buf) // self._channels
            cdata = (ctypes.c_float * len(buf))()
            cdata[:] = buf

        else:
            if len(data) != self._channels:
                raise ValueError('Expected {} output channels, got {}'.format(
                    self._channels, len(data)))
            frame_count = len(data[0])
            cdata = (ctypes.c_float * (frame_count * self._channels))()
            for ch in range(self._channels):
                if len(data[ch]) != frame_count:
                    raise ValueError('Output channel buffer lengths do not match')
                cdata[ch::self._channels] = data[ch]

        _sndfile.sf_writef_float(self._sf, cdata, frame_count)


class SndFileW(_SndFileWBase):

    def __init__(self,
                 fname,
                 format='wav',
                 rate=48000,
                 channels=2,
                 use_float=False,
                 bits=16):
        """Create a new writable audio file.

        Arguments:
        fname -- Output file name.

        Optional arguments:
        format    -- Output file format.
        rate      -- Audio rate.
        channels  -- Number of output channels.
        use_float -- Use floating-point frames.
        bits      -- Bits per item.

        """
        super().__init__()
        self._channels = channels

        info = self._get_validated_sf_info(format, rate, channels, use_float, bits)

        self._sf = _sndfile.sf_open(bytes(fname, encoding='utf-8'), SFM_WRITE, info)
        if not self._sf:
            err_cstr = _sndfile.sf_strerror(None)
            raise SndFileError('Could not create file {}: {}'.format(
                fname, str(err_cstr, encoding='utf-8')))

        if not use_float:
            _sndfile.sf_command(self._sf, SFC_SET_CLIPPING, None, SF_TRUE)

    def __del__(self):
        self.close()


class SndFileWMem(_SndFileWBase):

    def __init__(self,
                 format='wav',
                 rate=48000,
                 channels=2,
                 use_float=False,
                 bits=16):
        """Create a new writable audio stream to memory.

        Optional arguments:
        format    -- Output file format.
        rate      -- Audio rate.
        channels  -- Number of output channels.
        use_float -- Use floating-point frames.
        bits      -- Bits per item.

        """
        super().__init__()
        self._channels = channels

        self._bytes = bytearray()

        vio = self._get_virtual_io_info()
        info = self._get_validated_sf_info(format, rate, channels, use_float, bits)

        self._sf = _sndfile.sf_open_virtual(vio, SFM_WRITE, info, None)
        if not self._sf:
            err_cstr = _sndfile.sf_strerror(None)
            raise SndFileError('Could not set up data access: {}'.format(
                str(err_cstr, encoding='utf-8')))

        if not use_float:
            _sndfile.sf_command(self._sf, SFC_SET_CLIPPING, None, SF_TRUE)

    def get_file_contents(self):
        return self._bytes

    def __del__(self):
        self.close()


class SndFileError(Exception):
    """Class for libsndfile-related errors.

    """


_sf_count_t = ctypes.c_int64

class _SF_INFO(ctypes.Structure):
    _fields_ = [('frames', _sf_count_t),
                ('samplerate', ctypes.c_int),
                ('channels', ctypes.c_int),
                ('format', ctypes.c_int),
                ('sections', ctypes.c_int),
                ('seekable', ctypes.c_int)]


_sf_vio_get_filelen = ctypes.CFUNCTYPE(
        _sf_count_t,
        ctypes.c_void_p) # user_data

_sf_vio_seek = ctypes.CFUNCTYPE(
        _sf_count_t,
        _sf_count_t,
        ctypes.c_int,
        ctypes.c_void_p) # user_data

_sf_vio_read = ctypes.CFUNCTYPE(
        _sf_count_t,
        ctypes.POINTER(ctypes.c_char),
        _sf_count_t,
        ctypes.c_void_p) # user_data

_sf_vio_write = ctypes.CFUNCTYPE(
        _sf_count_t,
        ctypes.POINTER(ctypes.c_char),
        _sf_count_t,
        ctypes.c_void_p) # user_data

_sf_vio_tell = ctypes.CFUNCTYPE(
        _sf_count_t,
        ctypes.c_void_p) # user_data

class _SF_VIRTUAL_IO(ctypes.Structure):
    _fields_ = [('get_filelen', _sf_vio_get_filelen),
                ('seek', _sf_vio_seek),
                ('read', _sf_vio_read),
                ('write', _sf_vio_write),
                ('tell', _sf_vio_tell)]


_sndfile = ctypes.CDLL(ctypes.util.find_library('sndfile'))

_sndfile.sf_format_check.argtypes = [ctypes.POINTER(_SF_INFO)]
_sndfile.sf_format_check.restype = ctypes.c_int

_sndfile.sf_open.argtypes = [
        ctypes.c_char_p,
        ctypes.c_int,
        ctypes.POINTER(_SF_INFO)]
_sndfile.sf_open.restype = ctypes.c_void_p

_sndfile.sf_open_virtual.argtypes = [
        ctypes.POINTER(_SF_VIRTUAL_IO),
        ctypes.c_int,
        ctypes.POINTER(_SF_INFO),
        ctypes.c_void_p] # user_data
_sndfile.sf_open_virtual.restype = ctypes.c_void_p

_sndfile.sf_strerror.argtypes = [ctypes.c_void_p]
_sndfile.sf_strerror.restype = ctypes.c_char_p

_sndfile.sf_command.argtypes = [
        ctypes.c_void_p,
        ctypes.c_int,
        ctypes.c_void_p,
        ctypes.c_int]
_sndfile.sf_command.restype = ctypes.c_int

_sndfile.sf_readf_int.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_int),
        _sf_count_t]
_sndfile.sf_readf_int.restype = _sf_count_t

_sndfile.sf_readf_float.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_float),
        _sf_count_t]
_sndfile.sf_readf_float.restype = _sf_count_t

_sndfile.sf_writef_float.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_float),
        _sf_count_t]
_sndfile.sf_writef_float.restype = _sf_count_t

_sndfile.sf_close.argtypes = [ctypes.c_void_p]
_sndfile.sf_close.restype = ctypes.c_int


