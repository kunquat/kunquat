# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
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
                if stripped_path in self._contents:
                    raise KunquatFileError('Duplicate entry: {}'.format(stripped_path))

                value = zfile.read(entry)
                if stripped_path.endswith('.json'):
                    decoded = json.loads(str(value, encoding='utf-8'))
                else:
                    decoded = value

                self._contents[stripped_path] = decoded

                self._loading_progress = (i + 1) / entry_count

        finally:
            zfile.close()

    def get_contents(self):
        return self._contents


