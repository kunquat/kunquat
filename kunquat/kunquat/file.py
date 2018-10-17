# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

"""An interface for accessing Kunquat files.

"""

import ctypes
import json
import zipfile

from .kunquat import Kunquat, KunquatError


# Flags that specify what data should be kept in memory for multiple retrievals
KQTFILE_KEEP_NONE           = 0
KQTFILE_KEEP_RENDER_DATA    = 1 << 0
KQTFILE_KEEP_PLAYER_DATA    = 1 << 1
KQTFILE_KEEP_INTERFACE_DATA = 1 << 2
KQTFILE_KEEP_ALL_DATA       = (
        KQTFILE_KEEP_RENDER_DATA |
        KQTFILE_KEEP_PLAYER_DATA |
        KQTFILE_KEEP_INTERFACE_DATA)


def _get_file_type_desc(key_prefix):
    kqt_prefixes = {
        'kqtc00': 'Kunquat module',
        'kqti00': 'Kunquat instrument or effect',
    }
    return kqt_prefixes.get(key_prefix)


'''
class _KqtArchiveFile():

    _KEEP_MAP = {
        KQT_KEEP_RENDER_DATA    : lambda x: x.split('/')[-1].startswith('p_'),
        KQT_KEEP_PLAYER_DATA    : (
            lambda x: x.split('/')[-1].startswith('m_') or (x == 'album/p_tracks.json')),
        KQT_KEEP_INTERFACE_DATA : lambda x: x.split('/')[-1].startswith('i_'),
    }

    def __init__(self, key_prefix, path, keep_flags):
        self._key_prefix = key_prefix
        self._path = path
        self._keep_flags = keep_flags
        self._loading_progress = 0
        self._stored_entries = {}

    def _remove_prefix(self, arc_path):
        preparts = self._key_prefix.split('/')
        keyparts = arc_path.split('/')
        removed_parts = []
        for pp in preparts:
            kp = keyparts.pop(0)
            removed_parts.append(kp)
            if pp != kp:
                unexpected_prefix = '/'.join(removed_parts)
                unexpected_desc = _get_file_type_desc(unexpected_prefix)
                expected_desc = _get_file_type_desc(self._key_prefix)
                assert expected_desc
                assert unexpected_desc != expected_desc
                if unexpected_desc:
                    msg = 'File is a {}, not a {}'.format(unexpected_desc, expected_desc)
                else:
                    msg = 'Unexpected key prefix: {}'.format(unexpected_prefix)
                raise KunquatFileError(msg)

        if not keyparts:
            msg = 'File contains the magic ID {} as a regular file'.format(
                    self._key_prefix)
            raise KunquatFileError(msg)
        elif '.' not in keyparts[-1]:
            msg = 'The final element of key {} does not contain a period'.format(
                    '/'.join((self._key_prefix, *keyparts)))
            raise KunquatFileError(msg)

        return '/'.join(keyparts)

    def _keep_entry(self, key):
        last = key.split('/')[-1]
        for keep, match in self._KEEP_MAP.items():
            if (self._keep_flags & keep) != 0 and match(key):
                return True
        return False

    def _get_entries(self, zfile):
        data_found = False

        try:
            entries = [e for e in zfile.infolist() if not e.filename.endswith('/')]

            entry_count = len(entries)
            for i, entry in enumerate(entries):
                path = entry.filename
                key = self._remove_prefix(path)
                data_found = True

                try:
                    value = zfile.read(entry)
                except zipfile.BadZipFile as e:
                    raise KunquatFileError('Error while loading {}: {}'.format(
                        key, str(e)))

                if key.endswith('.json'):
                    try:
                        decoded = json.loads(str(value, encoding='utf-8'))
                    except ValueError as e:
                        msg = 'Invalid JSON data in {}: {}'.format(
                                key, str(e).capitalize())
                        raise KunquatFileError(msg)
                else:
                    decoded = value

                if self._keep_entry(key):
                    self._stored_entries[key] = decoded
                yield (key, decoded)

                self._loading_progress = (i + 1) / entry_count

        finally:
            zfile.close()

        if not data_found:
            type_desc = _get_file_type_desc(self._key_prefix)
            raise KunquatFileError('File contains no {} data'.format(type_desc))

    def get_entries(self):
        self._stored_entries = {}
        self._loading_progress = 0

        try:
            zfile = zipfile.ZipFile(self._path, mode='r')
        except OSError as e:
            raise KunquatFileError(str(e))
        except zipfile.BadZipFile:
            raise KunquatFileError('File is not a valid Kunquat file')

        return self._get_entries(zfile)

    def get_loading_progress(self):
        return self._loading_progress

    def get_stored_entries(self):
        return self._stored_entries.items()

    def try_get_editor_version(self):
        try:
            with zipfile.ZipFile(self._path, mode='r') as zfile:
                entries = [e for e in zfile.infolist() if not e.filename.endswith('/')]
                if entries:
                    first_key = entries[0].filename
                    maybe_magic_id = first_key.split('/')[0]

                    entry = zfile.getinfo(
                            '{}/m_editor_version.json'.format(maybe_magic_id))
                    value = zfile.read(entry)
                    decoded = json.loads(str(value, encoding='utf-8'))
                    return decoded
        except (KeyError, OSError, ValueError, zipfile.BadZipFile):
            pass

        return None
'''


class KqtFile():

    """A class for reading Kunquat module files.

    Public methods:
    load_into_handle       -- Load file contents into a Kunquat instance.
    load_into_handle_steps -- Load file contents into a Kunquat instance.
    get_entries            -- Get all entries of the module.
    get_loading_progress   -- Get normalised loading progress.
    get_stored_entries     -- Get entries stored in this object.

    """

    def __init__(self, kqt, keep_flags=KQTFILE_KEEP_PLAYER_DATA):
        """Create a new instance for reading .kqt files.

        Arguments:
        kqt -- The Kunquat instance associated with this KqtFile
               instance.

        Optional arguments:
        keep_flags -- Flags that specify which entries to keep inside
                      this object for permanent read access. See
                      get_stored_entries() for more details.

        """
        self._kqt = kqt
        self._module = _kqtfile.kqt_new_Module_with_handle(self._kqt.get_handle())
        _kqtfile.kqt_Module_set_keep_flags(self._module, keep_flags)

        self._path = None

    def _open_file(self, path):
        self._path = path
        _kqtfile.kqt_Module_open_file(self._module, bytes(path, encoding='utf-8'))

    def _close_file(self):
        _kqtfile.kqt_Module_close_file(self._module)
        self._path = None

    def load(self, path):
        """Load the Kunquat module.

        """
        for _ in self.load_steps(path):
            pass

    def load_steps(self, path):
        """Load the Kunquat module in steps.

        Return value:
        A generator that yields after every step of the loading
        procedure.

        """
        self._open_file(path)
        while _kqtfile.kqt_Module_load_step(self._module):
            yield
        self._close_file()

    def get_loading_progress(self):
        """Get current loading progress.

        Return value:
        Current progress normalised to the range [0, 1].

        """
        return _kqtfile.kqt_Module_get_loading_progress(self._module)

    def get_kept_entry_count(self):
        """Get the number of kept entries in the Kunquat module file.

        Return value:
        The number of entries.

        """
        return _kqtfile.kqt_Module_get_kept_entry_count(self._module)

    def get_kept_entries(self):
        """Get kept entries of the Kunquat module file.

        Return value:
        A generator that yields 2-tuples of format (k, v) where k is
        the key of the entry and v is the associated value.

        """
        entry_count = _kqtfile.kqt_Module_get_kept_entry_count(self._module)
        entry_keys = _kqtfile.kqt_Module_get_kept_keys(self._module)
        entry_sizes = _kqtfile.kqt_Module_get_kept_entry_sizes(self._module)
        entries = _kqtfile.kqt_Module_get_kept_entries(self._module)
        for i in range(entry_count):
            key = str(entry_keys[i], encoding='utf-8')
            vdata = entries[i][:entry_sizes[i]]
            value = bytes(vdata)
            if key.endswith('.json'):
                value = json.loads(str(value, encoding='utf-8'))
            yield (key, value)

    def try_get_editor_version(self):
        try:
            with zipfile.ZipFile(self._path, mode='r') as zfile:
                entries = [e for e in zfile.infolist() if not e.filename.endswith('/')]
                if entries:
                    first_key = entries[0].filename
                    maybe_magic_id = first_key.split('/')[0]

                    entry = zfile.getinfo(
                            '{}/m_editor_version.json'.format(maybe_magic_id))
                    value = zfile.read(entry)
                    decoded = json.loads(str(value, encoding='utf-8'))
                    return decoded
        except (KeyError, OSError, ValueError, zipfile.BadZipFile):
            pass

    def __del__(self):
        _kqtfile.kqt_del_Module(self._module)
        self._module = 0


class KunquatFileError(KunquatError):
    """Error indicating that a Kunquat file is invalid."""


_kqtfile = ctypes.CDLL('libkunquatfile.so')

kqt_Handle = ctypes.c_int
kqt_Module = ctypes.c_int

_kqtfile.kqtfile_load_module.argtypes = [ctypes.c_char_p, ctypes.c_int]
_kqtfile.kqtfile_load_module.restype = kqt_Handle

_kqtfile.kqt_new_Module_with_handle.argtypes = [kqt_Handle]
_kqtfile.kqt_new_Module_with_handle.restype = kqt_Module

_kqtfile.kqt_Module_get_error.argtypes = [kqt_Module]
_kqtfile.kqt_Module_get_error.restype = ctypes.c_char_p

_kqtfile.kqt_Module_clear_error.argtypes = [kqt_Module]
_kqtfile.kqt_Module_clear_error.restype = None

_kqtfile.kqt_Module_set_keep_flags.argtypes = [kqt_Module, ctypes.c_int]
_kqtfile.kqt_Module_set_keep_flags.restype = ctypes.c_int

_kqtfile.kqt_Module_open_file.argtypes = [kqt_Module, ctypes.c_char_p]
_kqtfile.kqt_Module_open_file.restype = ctypes.c_int

_kqtfile.kqt_Module_load_step.argtypes = [kqt_Module]
_kqtfile.kqt_Module_load_step.restype = ctypes.c_int

_kqtfile.kqt_Module_get_loading_progress.argtypes = [kqt_Module]
_kqtfile.kqt_Module_get_loading_progress.restype = ctypes.c_double

_kqtfile.kqt_Module_close_file.argtypes = [kqt_Module]
_kqtfile.kqt_Module_close_file.restype = None

_kqtfile.kqt_Module_get_kept_entry_count.argtypes = [kqt_Module]
_kqtfile.kqt_Module_get_kept_entry_count.restype = ctypes.c_long

_kqtfile.kqt_Module_get_kept_keys.argtypes = [kqt_Module]
_kqtfile.kqt_Module_get_kept_keys.restype = ctypes.POINTER(ctypes.c_char_p)

_kqtfile.kqt_Module_get_kept_entry_sizes.argtypes = [kqt_Module]
_kqtfile.kqt_Module_get_kept_entry_sizes.restype = ctypes.POINTER(ctypes.c_long)

_kqtfile.kqt_Module_get_kept_entries.argtypes = [kqt_Module]
_kqtfile.kqt_Module_get_kept_entries.restype = (
        ctypes.POINTER(ctypes.POINTER(ctypes.c_ubyte)))

_kqtfile.kqt_Module_free_kept_entries.argtypes = [kqt_Module]
_kqtfile.kqt_Module_free_kept_entries.restype = ctypes.c_int

_kqtfile.kqt_del_Module.argtypes = [kqt_Module]
_kqtfile.kqt_del_Module.restype = None


