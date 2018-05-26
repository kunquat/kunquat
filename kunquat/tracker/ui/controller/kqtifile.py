# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import json
import zipfile

from kunquat.kunquat.file import KunquatFileError


class KqtiFile():

    def __init__(self, path):
        self._path = path
        self._contents = None
        self._loading_progress = 0

    def get_path(self):
        return self._path

    def get_loading_progress(self):
        return self._loading_progress

    def get_read_steps(self):
        au_prefix = 'kqti00'
        self._contents = {}

        try:
            zfile = zipfile.ZipFile(self._path, mode='r')
        except OSError as e:
            raise KunquatFileError(str(e))
        except zipfile.BadZipFile:
            raise KunquatFileError('File is not a valid Kunquat file')

        try:
            entries = [e for e in zfile.infolist() if not e.filename.endswith('/')]

            entry_count = len(entries)
            for i, entry in enumerate(entries):
                yield
                path = entry.filename
                path_components = path.split('/')
                if path_components[0] != au_prefix:
                    raise KunquatFileError(
                            'Invalid magic ID: {}'.format(path_components[0]))

                stripped_path = '/'.join(path_components[1:])
                if not stripped_path:
                    raise KunquatFileError(
                            'File contains the magic ID {} as a regular file'.format(
                                au_prefix))
                if '.' not in path_components[-1]:
                    msg = 'The final element of key {} does not contain a period'.format(
                                stripped_path)
                    raise KunquatFileError(msg)
                if stripped_path in self._contents:
                    raise KunquatFileError('Duplicate entry: {}'.format(stripped_path))

                try:
                    value = zfile.read(entry)
                except zipfile.BadZipFile as e:
                    raise KunquatFileError('Error while loading key {}: {}'.format(
                        stripped_path, str(e)))

                if stripped_path.endswith('.json'):
                    decoded = json.loads(str(value, encoding='utf-8'))
                else:
                    decoded = value

                self._contents[stripped_path] = decoded

                self._loading_progress = (i + 1) / entry_count

            if not self._contents:
                raise KunquatFileError('File contains no Kunquat audio unit data')

        finally:
            zfile.close()

    def get_contents(self):
        return self._contents

    def try_get_editor_version(self):
        # TODO: copied from _KqtArchiveFile; share code
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


