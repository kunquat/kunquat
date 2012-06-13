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

import errno
import os
from tempfile import mkdtemp

import store


class Storage(object):
    '''
    >>> path = mkdtemp()
    >>> p = Storage(path)
    >>> s = p.new_store()
    >>> s._path in p.list_projects()
    True
    >>> p.collect_trash()
    >>> s._path in p.list_projects()
    False
    '''

    PERMANENT = 'pmt'
    TEMPORARY = 'tmp'

    def __init__(self, path, create=False):
        self.path = path
        if create:
            try:
                os.makedirs(self.path)
            except OSError as err:
                if err.errno != errno.EEXIST:
                    raise
        self.collect_trash()

    def new_store(self):
        sid = mkdtemp(dir = self.path, prefix = self.TEMPORARY)
        return store.Store(sid)

    def list_projects(self):
        dirs = os.listdir(self.path)
        paths = [os.path.join(self.path, d) for d in dirs]
        return paths

    def collect_trash(self):
        for path in self.list_projects():
            try:
                os.rmdir(path)
            except:
                pass

    def open(self, path):
        return self.new_store()



if __name__ == "__main__":
    import doctest
    doctest.testmod()
