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

from store import Store


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

    def __init__(self, path, prefix='', create=False):
        self.path = path
        self.prefix = prefix
        if create:
            try:
                os.makedirs(self.path)
            except OSError as err:
                if err.errno != errno.EEXIST:
                    raise
        self.collect_trash()

    def _new_store(self, callbacks=[]):
        sid = mkdtemp(dir = self.path, prefix = self.TEMPORARY)
        new = Store(sid, callbacks)
        return new

    def get_store(self, path=None, callbacks=[]):
        store = self._new_store(callbacks)
        if path:
            store.from_path(path, prefix=self.prefix)

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



if __name__ == "__main__":
    import doctest
    doctest.testmod()
