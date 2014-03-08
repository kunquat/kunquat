# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import MutableMapping


class Store(MutableMapping):

    def __init__(self):
        self._content = dict()
        self._audio_engine = None

    def set_audio_engine(self, audio_engine):
        self._audio_engine = audio_engine

    def put(self, transaction):
        self._audio_engine.set_data(transaction)

        # TODO: do this after we have received confirmation from audio engine
        self._content.update(transaction)
        for (key, value) in transaction.iteritems():
            if value == None:
                del self._content[key]

    def __getitem__(self, key):
        return self._content[key]

    def __setitem__(self, key, value):
        transaction = {key: value}
        self.put(transaction)

    def __delitem__(self, key):
        transaction = {key: None}
        self.put(transaction)

    def __iter__(self):
        return self._content.iterkeys()

    def __len__(self):
        return len(self._content)


