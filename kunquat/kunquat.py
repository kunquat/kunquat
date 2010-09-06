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

"""A library for accessing Kunquat music data.

This module provides interfaces for inspecting, mixing and modifying
Kunquat compositions.

Classes:
RHandle   -- A read-only interface for kqt files.
RWHandle  -- A read/write interface for composition directories.
RWCHandle -- An interface for composition projects.

Exceptions:
KunquatError         -- The base class for Kunquat errors.
KunquatArgumentError -- An error for most invalid argument errors.
KunquatFormatError   -- An error for indicating invalid music data.
KunquatMemoryError   -- An error for indicating memory allocation
                        failure.
KunquatResourceError -- An error for resource access errors.

"""

import ctypes
import json

__all__ = ['RHandle', 'RWHandle', 'RWCHandle',
           'KunquatError', 'KunquatArgumentError',
           'KunquatFormatError', 'KunquatMemoryError',
           'KunquatResourceError']


class RHandle(object):

    """Handle for accessing composition (kqt) files in read-only mode.

    Every Kunquat composition is accessed through a handle that is an
    RHandle -- possibly an instance of a subclass of RHandle.  The
    RHandle is used for mixing the composition and/or retrieving data
    from the composition.

    Public methods:
    __getitem__  -- Dictionary-like composition data retrieval.
    get_duration -- Calculate the length of a subsong.
    mix          -- Mix audio data.
    trigger      -- Trigger an event.

    Public instance variables:
    buffer_size -- Mixing buffer size.
    mixing_rate -- Mixing rate.
    nanoseconds -- The current position in nanoseconds.
    subsong     -- The current subsong (None or [0,255]).

    """

    def __init__(self, path, mixing_rate=48000):
        """Create a new RHandle.

        Arguments:
        path -- The path of a kqt file.  A kqt file name typically has
                the extension .kqt, possibly succeeded by an extension
                indicating a compression format.

        Optional arguments:
        mixing_rate -- Mixing rate in frames per second.  Typical
                       values include 44100 ("CD quality") and 48000
                       (the default).

        Exceptions:
        KunquatArgumentError -- path is None or mixing_rate is not
                                positive.
        KunquatFormatError   -- The file in path is not a valid Kunquat
                                composition.
        KunquatResourceError -- Reading the input failed for a reason
                                other than the data being invalid.

        """
        if '_handle' not in self.__dict__:
            self._handle = _kunquat.kqt_new_Handle_r(path)
            if not self._handle:
                raise _get_error(json.loads(
                                 _kunquat.kqt_Handle_get_error(None)))
        self._subsong = None
        self._nanoseconds = 0
        self._buffer_size = _kunquat.kqt_Handle_get_buffer_size(self._handle)
        if mixing_rate <= 0:
            raise KunquatArgumentError('Mixing rate must be positive')
        self.mixing_rate = mixing_rate

    def __getitem__(self, key):
        """Get data from the handle based on a key.

        Arguments:
        key -- The key of the data in the composition.  A key consists
               of one or more textual elements separated by forward
               slashes ('/').  The last element is the only one that
               is allowed and required to contain a period.  Examples:
               'p_composition.json'
               'pat_000/ccol_00/p_channel_events.json'
               'ins_01/kqtiXX/p_instrument.json'
               The 'XX' in the last example should be written
               literally.  It is expanded to the file format version
               number behind the scenes.

        Return value:
        The data associated with the key if found, otherwise None.

        Exceptions:
        KunquatArgumentError -- The key is not valid.
        KunquatResourceError -- Retrieving the data failed.  This can
                                usually occur only with subclasses of
                                RHandle.

        """
        length = _kunquat.kqt_Handle_get_data_length(self._handle, key)
        if length <= 0:
            return None
        cdata = _kunquat.kqt_Handle_get_data(self._handle, key)
        data = cdata[:length]
        _kunquat.kqt_Handle_free_data(self._handle, cdata)
        return ''.join(chr(ch) for ch in data)

    @property
    def subsong(self):
        """The current subsong ([0,255], or None for all subsongs)"""
        return self._subsong

    @subsong.setter
    def subsong(self, value):
        """Set the subsong number.

        Note that setting the subsong number also resets nanoseconds to
        zero.

        """
        subsong = value
        if subsong is None:
            subsong = -1
        _kunquat.kqt_Handle_set_position(self._handle, subsong, 0)
        self._subsong = value
        self._nanoseconds = 0

    @property
    def nanoseconds(self):
        """The current position in nanoseconds."""
        return self._nanoseconds

    @nanoseconds.setter
    def nanoseconds(self, value):
        """Set the subsong position in nanoseconds.

        Note that setting a new nanoseconds value may take a while for
        very large values.

        """
        subsong = self.subsong
        if subsong is None:
            subsong = -1
        _kunquat.kqt_Handle_set_position(self._handle, subsong, value)
        self._nanoseconds = value

    @property
    def mixing_rate(self):
        """Mixing rate in frames per second."""
        return self._mixing_rate

    @mixing_rate.setter
    def mixing_rate(self, value):
        """Set the mixing rate.

        Typical values include 44100 ("CD quality") and 48000 (the
        default).

        """
        _kunquat.kqt_Handle_set_mixing_rate(self._handle, value)
        self._mixing_rate = value

    @property
    def buffer_size(self):
        """Mixing buffer size in frames."""
        return self._buffer_size

    @buffer_size.setter
    def buffer_size(self, value):
        """Set the mixing buffer size.

        This value sets the maximum (and default) length for buffers
        returned by the mix method.

        """
        _kunquat.kqt_Handle_set_buffer_size(self._handle, value)
        self._buffer_size = value

    def get_duration(self, subsong=None):
        """Count the duration of the composition in nanoseconds.

        Arguments:
        subsong -- The subsong of which length is to be calculated.
                   If this parameter is omitted, the function returns
                   the total length of all subsongs.

        Return value:
        The duration in nanoseconds.

        Exceptions:
        KunquatArgumentError -- The subsong number is not valid.

        """
        if subsong is None:
            subsong = -1
        return _kunquat.kqt_Handle_get_duration(self._handle, subsong)

    def mix(self, frame_count=None):
        """Mix audio according to the state of the handle.

        Optional arguments:
        frame_count -- The number of frames to be mixed.  The default
                       value is self.buffer_size.

        Exceptions:
        KunquatArgumentError -- frame_count is not positive.

        Returns:
        A pair containing audio data for, respectively, the left and
        the right output channel.  Buffers shorter than frame_count
        frames indicate that the end has been reached.

        """
        if not frame_count:
            frame_count = self._buffer_size
        mixed = _kunquat.kqt_Handle_mix(self._handle, frame_count)
        cbuf_left = _kunquat.kqt_Handle_get_buffer(self._handle, 0)
        cbuf_right = _kunquat.kqt_Handle_get_buffer(self._handle, 1)
        self._nanoseconds = _kunquat.kqt_Handle_get_position(self._handle)
        return cbuf_left[:mixed], cbuf_right[:mixed]

    def trigger(self, channel, event):
        """Trigger an event.

        Arguments:
        channel -- The channel where the event takes place. The channel
                   number is >= 0 and < 64 for regular channels, and -1
                   for the global channel.
        event -- The event description in JSON format.  The description
                 is a pair (list with two elements) with the event name
                 as the first element and its argument list as the
                 second element.  Example: '["Cn+", [300]]' (Note On at
                 300 cents above A4, i.e. C5 in 12-tone Equal
                 Temperament).

        """
        _kunquat.kqt_Handle_trigger(self._handle, channel, event)

    def __del__(self):
        if self._handle:
            _kunquat.kqt_del_Handle(self._handle)
        self._handle = None


class RWHandle(RHandle):

    """Handle for accessing composition directories in read/write mode.

    The RWHandle is used for accessing extracted Kunquat composition
    directories.  It does not support loading kqt files.  As an
    extension to RHandle, it is capable of modifying composition data.

    The RWHandle is mainly designed for applications that modify
    composition metadata.  It is not recommended for editor
    applications.  These applications should use RWCHandle instead.

    Public methods:
    __setitem__ -- Dictionary-like composition data modification.

    """

    def __init__(self, path, mixing_rate=48000):
        """Create a new RWHandle.

        Arguments:
        path -- The path to the extracted Kunquat composition
                directory.  This directory is called 'kqtcXX'
                where 'XX' is the version number of the format.  In
                this case, the real path name should be used, i.e.
                the format version number should not be substituted
                with 'XX'.

        Optional arguments:
        mixing_rate -- Mixing rate in frames per second.  Typical
                       values include 44100 ("CD quality") and 48000
                       (the default).

        """
        if '_handle' not in self.__dict__:
            self._handle = _kunquat.kqt_new_Handle_rw(path)
            if not self._handle:
                raise _get_error(json.loads(
                                 _kunquat.kqt_Handle_get_error(None)))
        RHandle.__init__(self, path, mixing_rate)

    def __setitem__(self, key, value):
        """Set data in the handle.

        Arguments:
        key   -- The key of the data in the composition.  This refers
                 to the same data as the key argument of __getitem__
                 and the same formatting rules apply.
        value -- The data to be set.

        Exceptions:
        KunquatArgumentError -- The key is not valid.
        KunquatFormatError   -- The data is not valid.  Only the data
                                that audibly affects mixing is
                                validated.

        """
        data = buffer(value)
        cdata = (ctypes.c_byte * len(data))()
        cdata[:] = [ord(b) for b in data][:]
        _kunquat.kqt_Handle_set_data(self._handle,
                                     key,
                                     ctypes.cast(cdata,
                                         ctypes.POINTER(ctypes.c_byte)),
                                     len(data))


class RWCHandle(RWHandle):

    """Handle for accessing composition projects with a state store.

    The RWCHandle extends the RWHandle with a journaling mechanism.  It
    enables the user to commit changes made in the composition state.
    A committed version of a composition can always be restored in case
    the program execution is abruptly terminated.

    Public methods:
    commit -- Commit the changes made in the handle.

    """

    def __init__(self, path, mixing_rate=48000):
        """Create a new RWCHandle.

        Arguments:
        path -- The path to the Kunquat composition project directory.
                Normally, this directory contains the subdirectories
                "committed" and "workspace", although new project
                directories may be empty.

        Optional arguments:
        mixing_rate -- Mixing rate in frames per second.  Typical
                       values include 44100 ("CD quality") and 48000
                       (the default).

        """
        if '_handle' not in self.__dict__:
            self._handle = _kunquat.kqt_new_Handle_rwc(path)
            if not self._handle:
                raise _get_error(json.loads(
                                 _kunquat.kqt_Handle_get_error(None)))
        RWHandle.__init__(self, path, mixing_rate)

    def commit(self):
        """Commit the changes made in the handle.

        Exceptions:
        KunquatFormatError   -- The project contains invalid data.
                                This usually means that the workspace
                                was modified manually or there's a bug
                                in Kunquat.
        KunquatResourceError -- File system access failed.

        If any exception is raised during the commit, it is usually a
        good idea to discard the handle and create a new one for the
        project.  This will initiate a recovery procedure that will
        restore a valid composition state.  Changes made after the last
        successful commit are possibly lost in this case, though.
        
        """
        _kunquat.kqt_Handle_commit(self._handle)


def _get_error(obj):
    if obj['type'] == 'ArgumentError':
        return KunquatArgumentError(obj)
    elif obj['type'] == 'FormatError':
        return KunquatFormatError(obj)
    elif obj['type'] == 'MemoryError':
        return KunquatMemoryError(obj)
    elif obj['type'] == 'ResourceError':
        return KunquatResourceError(obj)
    return KunquatError(obj)


def _error_check(result, func, arguments):
    chandle = arguments[0]
    error_str = _kunquat.kqt_Handle_get_error(chandle)
    if not error_str:
        return result
    _kunquat.kqt_Handle_clear_error(chandle)
    raise _get_error(json.loads(error_str))


class KunquatError(Exception):

    """Base class for errors in Kunquat."""

    def __init__(self, obj):
        self.obj = obj

    def __getitem__(self, key):
        return self.obj[key]

    def __str__(self):
        return self.obj['message']


class KunquatArgumentError(KunquatError):
    """Error indicating that a given method argument is invalid."""


class KunquatFormatError(KunquatError):
    """Error indicating that composition data is invalid."""


class KunquatMemoryError(KunquatError, MemoryError):
    """Error indicating that memory allocation failed."""


class KunquatResourceError(KunquatError):
    """Error indicating that an external service request has failed."""


_kunquat = ctypes.CDLL('libkunquat.so')

_kunquat.kqt_new_Handle_r.argtypes = [ctypes.c_char_p]
_kunquat.kqt_new_Handle_r.restype = ctypes.c_void_p
_kunquat.kqt_new_Handle_rw.argtypes = [ctypes.c_char_p]
_kunquat.kqt_new_Handle_rw.restype = ctypes.c_void_p
_kunquat.kqt_new_Handle_rwc.argtypes = [ctypes.c_char_p]
_kunquat.kqt_new_Handle_rwc.restype = ctypes.c_void_p
_kunquat.kqt_del_Handle.argtypes = [ctypes.c_void_p]

_kunquat.kqt_Handle_get_error.argtypes = [ctypes.c_void_p]
_kunquat.kqt_Handle_get_error.restype = ctypes.c_char_p
_kunquat.kqt_Handle_clear_error.argtypes = [ctypes.c_void_p]

_kunquat.kqt_Handle_get_data.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
_kunquat.kqt_Handle_get_data.restype = ctypes.POINTER(ctypes.c_byte)
_kunquat.kqt_Handle_get_data.errcheck = _error_check
_kunquat.kqt_Handle_get_data_length.argtypes = [ctypes.c_void_p,
                                                ctypes.c_char_p]
_kunquat.kqt_Handle_get_data_length.restype = ctypes.c_long
_kunquat.kqt_Handle_get_data_length.errcheck = _error_check
_kunquat.kqt_Handle_free_data.argtypes = [ctypes.c_void_p,
                                          ctypes.POINTER(ctypes.c_byte)]
_kunquat.kqt_Handle_free_data.restype = ctypes.c_int
_kunquat.kqt_Handle_free_data.errcheck = _error_check
_kunquat.kqt_Handle_set_data.argtypes = [ctypes.c_void_p,
                                         ctypes.c_char_p,
                                         ctypes.POINTER(ctypes.c_byte),
                                         ctypes.c_long]
_kunquat.kqt_Handle_set_data.restype = ctypes.c_int
_kunquat.kqt_Handle_set_data.errcheck = _error_check

_kunquat.kqt_Handle_commit.argtypes = [ctypes.c_void_p]
_kunquat.kqt_Handle_commit.restype = ctypes.c_int
_kunquat.kqt_Handle_commit.errcheck = _error_check

_kunquat.kqt_Handle_mix.argtypes = [ctypes.c_void_p,
                                    ctypes.c_long]
_kunquat.kqt_Handle_mix.restype = ctypes.c_long
_kunquat.kqt_Handle_mix.errcheck = _error_check
_kunquat.kqt_Handle_get_buffer.argtypes = [ctypes.c_void_p, ctypes.c_int]
_kunquat.kqt_Handle_get_buffer.restype = ctypes.POINTER(ctypes.c_float)
_kunquat.kqt_Handle_get_buffer.errcheck = _error_check

_kunquat.kqt_Handle_set_mixing_rate.argtypes = [ctypes.c_void_p,
                                                ctypes.c_long]
_kunquat.kqt_Handle_set_mixing_rate.restype = ctypes.c_int
_kunquat.kqt_Handle_set_mixing_rate.errcheck = _error_check
_kunquat.kqt_Handle_get_mixing_rate.argtypes = [ctypes.c_void_p]
_kunquat.kqt_Handle_get_mixing_rate.restype = ctypes.c_long
_kunquat.kqt_Handle_get_mixing_rate.errcheck = _error_check

_kunquat.kqt_Handle_set_buffer_size.argtypes = [ctypes.c_void_p,
                                                ctypes.c_long]
_kunquat.kqt_Handle_set_buffer_size.restype = ctypes.c_int
_kunquat.kqt_Handle_set_buffer_size.errcheck = _error_check
_kunquat.kqt_Handle_get_buffer_size.argtypes = [ctypes.c_void_p]
_kunquat.kqt_Handle_get_buffer_size.restype = ctypes.c_long
_kunquat.kqt_Handle_get_buffer_size.errcheck = _error_check

_kunquat.kqt_Handle_get_duration.argtypes = [ctypes.c_void_p, ctypes.c_int]
_kunquat.kqt_Handle_get_duration.restype = ctypes.c_longlong
_kunquat.kqt_Handle_get_duration.errcheck = _error_check
_kunquat.kqt_Handle_set_position.argtypes = [ctypes.c_void_p,
                                             ctypes.c_int,
                                             ctypes.c_longlong]
_kunquat.kqt_Handle_set_position.restype = ctypes.c_int
_kunquat.kqt_Handle_set_position.errcheck = _error_check
_kunquat.kqt_Handle_get_position.argtypes = [ctypes.c_void_p]
_kunquat.kqt_Handle_get_position.restype = ctypes.c_longlong
_kunquat.kqt_Handle_get_position.errcheck = _error_check

_kunquat.kqt_Handle_trigger.argtypes = [ctypes.c_void_p,
                                        ctypes.c_int,
                                        ctypes.c_char_p]
_kunquat.kqt_Handle_trigger.restype = ctypes.c_int
_kunquat.kqt_Handle_trigger.errcheck = _error_check


