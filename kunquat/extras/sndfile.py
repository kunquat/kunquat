# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

"""A wrapper for writing sound files through libsndfile.

"""

from __future__ import print_function
import ctypes


FORMAT_WAV  = 0x010000
FORMAT_AU   = 0x020000
FORMAT_FLAC = 0x170000

FORMAT_PCM_S8 = 0x0001
FORMAT_PCM_16 = 0x0002
FORMAT_PCM_24 = 0x0003
FORMAT_PCM_32 = 0x0004

FORMAT_PCM_U8 = 0x0005

FORMAT_FLOAT = 0x0006

SF_FALSE = 0
SF_TRUE = 1

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


class SndFileW(object):

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
        self._channels = channels
        self._sf = None
        fspec = formats_map[format]
        if use_float:
            fspec |= FORMAT_FLOAT
        else:
            if format == 'wav' and bits == 8:
                fspec |= FORMAT_PCM_U8
            else:
                fspec |= bits_map[bits]

        info = _SF_INFO(0, rate, channels, fspec, 0, 0)
        if not _sndfile.sf_format_check(info):
            num_mode = 'floating' if use_float else 'fixed'
            hdesc = '{}, {} point, {} bits'.format(format, num_mode, bits)
            raise ValueError('Unsupported format parameter combination:'
                    ' {}'.format(hdesc))

        self._sf = _sndfile.sf_open(fname, SFM_WRITE, info)
        if not self._sf:
            raise SndFileError('Couldn\'t create file {}: {}'.format(
                fname, _sndfile.sf_strerror(None)))

        if not use_float:
            _sndfile.sf_command(self._sf, SFC_SET_CLIPPING, None, SF_TRUE)

    def write(self, *data):
        """Write audio data.

        Arguments:
        data -- The output buffers, each channel as a separate
                argument.

        """
        if len(data) != self._channels:
            raise ValueError('Expected {} output channels, got {}'.format(
                self._channels, len(data)))

        frame_count = len(data[0])
        cdata = (ctypes.c_float * (frame_count * self._channels))()
        for ch in xrange(self._channels):
            if len(data[ch]) != frame_count:
                raise ValueError('Output channel buffer lengths do not match')
            cdata[ch::self._channels] = data[ch]

        _sndfile.sf_writef_float(self._sf, cdata, frame_count)

    def __del__(self):
        if self._sf:
            _sndfile.sf_close(self._sf)
        self._sf = None


class SndFileError(Exception):
    """Class for libsndfile-related errors.

    """


class _SF_INFO(ctypes.Structure):
    _fields_ = [('frames', ctypes.c_int64),
                ('samplerate', ctypes.c_int),
                ('channels', ctypes.c_int),
                ('format', ctypes.c_int),
                ('sections', ctypes.c_int),
                ('seekable', ctypes.c_int)]


_sndfile = ctypes.CDLL('libsndfile.so')

_sndfile.sf_format_check.argtypes = [ctypes.POINTER(_SF_INFO)]
_sndfile.sf_format_check.restype = ctypes.c_int

_sndfile.sf_open.argtypes = [
        ctypes.c_char_p,
        ctypes.c_int,
        ctypes.POINTER(_SF_INFO)]
_sndfile.sf_open.restype = ctypes.c_void_p

_sndfile.sf_strerror.argtypes = [ctypes.c_void_p]
_sndfile.sf_strerror.restype = ctypes.c_char_p

_sndfile.sf_command.argtypes = [
        ctypes.c_void_p,
        ctypes.c_int,
        ctypes.c_void_p,
        ctypes.c_int]
_sndfile.sf_command.restype = ctypes.c_int

_sndfile.sf_writef_float.argtypes = [
        ctypes.c_void_p,
        ctypes.POINTER(ctypes.c_float),
        ctypes.c_int64]
_sndfile.sf_writef_float.restype = ctypes.c_int64

_sndfile.sf_close.argtypes = [ctypes.c_void_p]
_sndfile.sf_close.restype = ctypes.c_int


