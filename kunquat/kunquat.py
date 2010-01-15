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

import ctypes


class RHandle(object):

    """Handle for accessing composition (kqt) files in read-only mode.

    Every Kunquat composition is accessed through a handle that is an
    RHandle -- possibly an instance of a subclass of RHandle.  The
    RHandle is used for mixing the composition and/or retrieving data
    from the composition.

    """

    def __init__(self, path):
        """Create a new RHandle.

        Arguments:
        path -- The path of a kqt file.  A kqt file name typically has
                the extension .kqt, possibly succeeded by an extension
                indicating a compression format.

        """
        if '_handle' not in self.__dict__:
            self._handle = _kunquat.kqt_new_Handle_r(path)
            if not self._handle:
                _raise_error(_kunquat.kqt_Handle_get_error(None))
        self.subsong = None
        self.nanoseconds = 0
        self.buffer_size = _kunquat.kqt_Handle_get_buffer_size(self._handle)

    def __getitem__(self, key):
        """Get data from the handle based on a key.

        Arguments:
        key -- The key of the data in the composition.  A key consists
               of one or more textual elements separated by forward
               slashes ('/').  The last element is the only one that
               is allowed and required to contain a period. Examples:
               'p_composition.json'
               'pat_000/vcol_00/p_voice_events.json'
               'ins_01/kunquatiXX/p_instrument.json'
               The 'XX' in the last example should be written exactly
               like this.  It is expanded to the file format version
               number behind the scenes.

        Return value:
        The data associated with the key if found, otherwise None.

        """
        length = _kunquat.kqt_Handle_get_data_length(self._handle, key)
        if length == 0:
            return None
        cdata = _kunquat.kqt_Handle_get_data(self._handle, key)
        data = cdata[:length]
        _kunquat.kqt_Handle_free_data(self._handle, cdata)
        return ''.join([chr(ch) for ch in data])

    def __setattr__(self, name, value):
        if name == 'subsong':
            subsong = value
            if subsong is None:
                subsong = -1
            _kunquat.kqt_Handle_set_position(self._handle, subsong, 0)
        elif name == 'nanoseconds':
            subsong = self.subsong
            if subsong is None:
                subsong = -1
            _kunquat.kqt_Handle_set_position(self._handle,
                                             subsong,
                                             value)
        elif name == 'buffer_size':
            _kunquat.kqt_Handle_set_buffer_size(self._handle, value)
        object.__setattr__(self, name, value)

    def get_duration(self, subsong=None):
        """Count the duration of the composition in nanoseconds.

        Arguments:
        subsong -- The subsong of which length is to be calculated.
                   If this parameter is omitted, the function returns
                   the total length of all subsongs.

        """
        if subsong is None:
            subsong = -1
        return _kunquat.kqt_Handle_get_duration(self._handle, subsong)

    def mix(self, frame_count, freq):
        """Mix audio according to the state of the handle.

        Arguments:
        frame_count -- The number of frames to be mixed.
        freq        -- The mixing frequency in frames per second.
                       Typical values are 44100 ("CD quality") and
                       48000.

        Returns:
        A pair containing audio data for, respectively, the left and
        the right output channel. Buffers shorter than frame_count
        frames indicate that the end has been reached.

        """
        mixed = _kunquat.kqt_Handle_mix(self._handle, frame_count, freq)
        cbuf_left = _kunquat.kqt_Handle_get_buffer(self._handle, 0)
        cbuf_right = _kunquat.kqt_Handle_get_buffer(self._handle, 1)
        return (cbuf_left[:mixed], cbuf_right[:mixed])

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

    """

    def __init__(self, path):
        """Create a new RWHandle.

        Arguments:
        path -- The path to the extracted Kunquat composition
                directory.  This directory is called kunquatcXX where
                XX is the version number of the format.  In this case,
                the real path name should be used, i.e. don't
                substitute the format version with XX.

        """
        if '_handle' not in self.__dict__:
            self._handle = _kunquat.kqt_new_Handle_rw(path)
            if not self._handle:
                _raise_error(_kunquat.kqt_Handle_get_error(None))
        RHandle.__init__(self, path)

    def __setitem__(self, key, value):
        """Set data in the handle.

        Arguments:
        key   -- The key of the data in the composition.  This refers
                 to the same data as the key argument of __getitem__
                 and the same formatting rules apply.
        value -- The data to be set.

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
    enables the user to commit to changes made in the composition
    state.  A committed version of a composition can always be restored
    in case the program execution is abruptly terminated.

    """

    def __init__(self, path):
        """Create a new RWCHandle.

        Arguments:
        path -- The path to the Kunquat composition project directory.
                Normally, this directory contains the subdirectories
                "committed" and "workspace", although new project
                directories can be empty.

        """
        if '_handle' not in self.__dict__:
            self._handle = _kunquat.kqt_new_Handle_rwc(path)
            if not self._handle:
                _raise_error(_kunquat.kqt_Handle_get_error(None))
        RWHandle.__init__(self, path)

    def commit():
        """Commits the changes made in the handle."""
        _kunquat.kqt_Handle_commit(self._handle)


def _raise_error(error_str):
    description = error_str[error_str.index(' ') + 1:]
    if error_str.startswith('ArgumentError:'):
        raise KunquatArgumentError(description)
    elif error_str.startswith('FormatError:'):
        raise KunquatFormatError(description)
    elif error_str.startswith('MemoryError:'):
        raise MemoryError(description)
    elif error_str.startswith('ResourceError:'):
        raise KunquatResourceError(description)
    raise KunquatError(error_str)


def _error_check(result, func, arguments):
    chandle = arguments[0]
    error_str = _kunquat.kqt_Handle_get_error(chandle)
    if not error_str:
        return result
    _kunquat.kqt_Handle_clear_error(chandle)
    _raise_error(error_str)


class KunquatError(Exception):
    """Base class for errors in Kunquat."""
    pass

class KunquatArgumentError(KunquatError):
    """Error indicating that a given method argument is invalid."""
    pass

class KunquatFormatError(KunquatError):
    """Error indicating that composition data is invalid."""
    pass

class KunquatResourceError(KunquatError):
    """Error indicating that an external service request has failed."""
    pass


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
                                    ctypes.c_long,
                                    ctypes.c_long]
_kunquat.kqt_Handle_mix.restype = ctypes.c_long
_kunquat.kqt_Handle_mix.errcheck = _error_check
_kunquat.kqt_Handle_get_buffer.argtypes = [ctypes.c_void_p, ctypes.c_int]
_kunquat.kqt_Handle_get_buffer.restype = ctypes.POINTER(ctypes.c_float)
_kunquat.kqt_Handle_get_buffer.errcheck = _error_check
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


