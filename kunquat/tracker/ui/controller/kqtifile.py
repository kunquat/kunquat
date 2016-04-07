# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import json
import tarfile


class KqtiFile():

    def __init__(self, path):
        self._path = path
        self._contents = None

    def get_path(self):
        return self._path

    def get_read_steps(self):
        au_prefix = 'kqti00'
        self._contents = {}

        tfile = tarfile.open(self._path, format=tarfile.USTAR_FORMAT)
        members = tfile.getmembers()
        for entry in members:
            yield
            path = entry.name
            path_components = path.split('/')
            if path_components[0] != au_prefix:
                raise NotImplementedError # TODO: invalid path
            if entry.isfile():
                stripped_path = '/'.join(path_components[1:])
                if not stripped_path:
                    raise NotImplementedError # TODO: regular file named as magic id
                if stripped_path in self._contents:
                    raise NotImplementedError # TODO: handle path duplicates

                value = tfile.extractfile(entry).read()
                if stripped_path.endswith('.json'):
                    decoded = json.loads(value)
                else:
                    decoded = value

                self._contents[stripped_path] = decoded

        tfile.close()

    def get_contents(self):
        return self._contents


