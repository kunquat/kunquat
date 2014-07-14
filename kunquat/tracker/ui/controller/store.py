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

from collections import deque, MutableMapping
from itertools import count


class Store(MutableMapping):

    def __init__(self):
        self._content = dict()
        self._audio_engine = None
        self._pending_validation = deque()
        self._transaction_ids = count()
        self._flush_callbacks = {}
        self._is_saving = False

    def set_audio_engine(self, audio_engine):
        self._audio_engine = audio_engine

    def put(self, transaction):
        assert not self._is_saving
        transaction_id = self._transaction_ids.next()
        self._audio_engine.set_data(transaction_id, transaction)
        self._pending_validation.append((transaction_id, transaction))

    def flush(self, callback):
        transaction_id = self._transaction_ids.next()
        self._audio_engine.set_data(transaction_id, None)
        self._flush_callbacks[transaction_id] = callback

    def confirm_valid_data(self, transaction_id):
        if transaction_id in self._flush_callbacks:
            self._flush_callbacks[transaction_id]()
            del self._flush_callbacks[transaction_id]
            return

        transaction = self._get_validated_transaction(transaction_id)
        self._content.update(transaction)
        for (key, value) in transaction.iteritems():
            if value == None:
                del self._content[key]

    def set_saving(self, enabled):
        self._is_saving = enabled

    def _get_validated_transaction(self, validated_id):
        transaction_id, transaction = self._pending_validation.popleft()
        assert transaction_id == validated_id
        return transaction

    def __getitem__(self, key):
        # If the key has non-validated changes, return the most recent one
        for _, transaction in reversed(self._pending_validation):
            if key in transaction:
                return transaction[key]

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


