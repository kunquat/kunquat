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


class Store():

    def __init__(self):
        self._content = dict()

    def put(self, transaction):
        self._content.update(transaction)

    def get(self, key):
        return self._content[key]

