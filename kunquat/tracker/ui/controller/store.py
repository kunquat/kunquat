# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import UserDict


class Store(UserDict.DictMixin):

    def __init__(self):
        self._content = dict()

    def put(self, transaction):
        self._content.update(transaction)

    def __getitem__(self, key):
        return self._content[key]

    def __setitem__(self, key, value):
        transaction = {key: value}
        self.put(transaction)

    def __delitem__(self, key):
        del self._content[key]

    def keys(self):
        return self._content.keys()

