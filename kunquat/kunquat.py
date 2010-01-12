

import ctypes


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
    """An error indicating that a given method argument was invalid."""
    pass

class KunquatFormatError(KunquatError):
    """An error indicating that composition data was invalid."""
    pass

class KunquatResourceError(KunquatError):
    """An error indicating that an external service request failed."""
    pass


class Rhandle(object):

    def __init__(self, path):
        self._handle = _kunquat.kqt_new_Handle_r(path)
        if not self._handle:
            _raise_error(_kunquat.kqt_Handle_get_error(None))
        self._subsong = -1
        self.position = 0

    def __getitem__(self, key):
        length = _kunquat.kqt_Handle_get_data_length(self._handle, key)
        if length == 0:
            return None
        cdata = _kunquat.kqt_Handle_get_data(self._handle, key)
        data = cdata[:length]
        _kunquat.kqt_Handle_free_data(self._handle, cdata)
        return ''.join([chr(ch) for ch in data])

    def set_position(self, position, subsong=None):
        if subsong is None:
            subsong = -1
        success = _kunquat.kqt_Handle_set_position(self._handle,
                                                   subsong, position)
        self._subsong = subsong

    def get_duration(self, subsong=None):
        if subsong is None:
            subsong = -1
        length = _kunquat.kqt_Handle_get_duration(self._handle, subsong)
        return length

    def mix(self, frame_count, freq):
        mixed = _kunquat.kqt_Handle_mix(self._handle, frame_count, freq)
        cbuf_left = _kunquat.kqt_Handle_get_buffer(self._handle, 0)
        cbuf_right = _kunquat.kqt_Handle_get_buffer(self._handle, 1)
        return (cbuf_left[:mixed], cbuf_right[:mixed])

    def __del__(self):
        if self._handle:
            _kunquat.kqt_del_Handle(self._handle)
        self._handle = None


class RWhandle(Rhandle):

    def __init__(self, path):
        self._handle = _kunquat.kqt_new_Handle_rw(path)
        if not self._handle:
            _raise_error(_kunquat.kqt_Handle_get_error(None))
        self._subsong = -1
        self.position = 0

    def __setitem__(self, key, value):
        data = buffer(value)
        cdata = (ctypes.c_byte * len(data))()
        cdata[:] = [ord(b) for b in data][:]
        _kunquat.kqt_Handle_set_data(self._handle,
                                     key,
                                     ctypes.cast(cdata,
                                         ctypes.POINTER(ctypes.c_byte)),
                                     len(data))


class RWChandle(RWhandle):

    def __init__(self, path):
        self._handle = _kunquat.kqt_new_Handle_rwc(path)
        if not self._handle:
            _raise_error(_kunquat.kqt_Handle_get_error(None))
        self._subsong = -1
        self.position = 0

    def commit():
        _kunquat.kqt_Handle_commit(self._handle)


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


