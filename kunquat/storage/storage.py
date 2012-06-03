

import errno
import os
from tempfile import mkdtemp

import store


class Storage(object):

    PERMANENT = 'pmt'

    def __init__(self, path, create=False):
        self.path = path
        if create:
            try:
                os.makedirs(self.path)
            except OSError as err:
                if err.errno != errno.EEXIST:
                    raise

    def new_store(self):
        sid = mkdtemp(dir = self.path, prefix = self.PERMANENT)
        return store.Store(sid)

    def open(self, path):
        return self.new_store()


