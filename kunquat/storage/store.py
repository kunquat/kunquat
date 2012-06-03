# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2012
#         Toni Ruottu, Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

class Store(object):

    def __init__(self, path):
        self._path = path
        self._memory = {}

    def __getitem__(self, key):
        return self._memory[key]

    def __setitem__(self, key, value):
        self._memory[key] = value

    def commit(self):
        pass

    def flush(self):
        pass

    def to_tar(self, path, key_prefix=''):
        pass

    def del_tree(self, key_prefix=''):
        pass

    def from_tar(self, path, key_prefix=''):
        pass


