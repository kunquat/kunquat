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

    def new_store(self):
        sid = mkdtemp(dir = self.path, prefix = self.TEMPORARY)
        return store.Store(sid)

    def open(self, path):
        return self.new_store()


