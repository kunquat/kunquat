# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2014-2018
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
        self._updater = None
        self._data_converters = None
        self._pending_validation = deque()
        self._transaction_ids = count()
        self._transaction_notifiers = {}
        self._flush_callbacks = {}
        self._is_saving = False
        self._is_modified = False

    def set_updater(self, updater):
        self._updater = updater

    def set_audio_engine(self, audio_engine):
        self._audio_engine = audio_engine

    def set_data_converters(self, data_converters):
        self._data_converters = data_converters

    def put(self, transaction, mark_modified=True, transaction_notifier=None):
        assert not self._is_saving
        transaction_id = next(self._transaction_ids)
        if transaction_notifier != None:
            self._transaction_notifiers[transaction_id] = transaction_notifier

        ver_transaction = { k: self._data_converters.wrap_with_latest_version(k, v)
                for (k, v) in transaction.items() }

        self._audio_engine.set_data(transaction_id, ver_transaction)
        self._pending_validation.append((transaction_id, ver_transaction))
        if mark_modified:
            if not self._is_modified:
                self._updater.signal_update_deferred('signal_store')
            self._is_modified = True

    def put_raw(self, entries):
        """Update store directly.

        This function assumes that the data is already stored and
        validated inside the Kunquat instance. Also, the transaction
        values are expected to contain the version information.

        """
        # This function assumes that the data is already stored and validated
        # in the Kunquat instance.
        assert not self._is_saving
        assert not self._pending_validation

        self._content.update(entries)
        for (key, value) in entries.items():
            if value == None:
                del self._content[key]

    def flush(self, callback):
        transaction_id = next(self._transaction_ids)
        self._audio_engine.set_data(transaction_id, None)
        self._flush_callbacks[transaction_id] = callback

    def update_transaction_progress(self, transaction_id, progress):
        if transaction_id in self._transaction_notifiers:
            self._transaction_notifiers[transaction_id](progress)
            if progress >= 1:
                del self._transaction_notifiers[transaction_id]

    def confirm_valid_data(self, transaction_id):
        if transaction_id in self._flush_callbacks:
            self._flush_callbacks[transaction_id]()
            del self._flush_callbacks[transaction_id]
            return

        transaction = self._get_validated_transaction(transaction_id)
        self._content.update(transaction)
        for (key, value) in transaction.items():
            if value == None:
                del self._content[key]

    def set_saving(self, enabled):
        self._is_saving = enabled

    def is_modified(self):
        return self._is_modified

    def clear_modified_flag(self):
        if self._is_modified:
            self._updater.signal_update_deferred('signal_store')
        self._is_modified = False

    def _get_validated_transaction(self, validated_id):
        transaction_id, transaction = self._pending_validation.popleft()
        assert transaction_id == validated_id
        return transaction

    def get_with_version(self, key):
        # If the key has non-validated changes, return the most recent one
        for _, transaction in reversed(self._pending_validation):
            if key in transaction:
                if transaction[key] == None:
                    # None won't be stored in validated contents, so act accordingly
                    raise KeyError(key)
                return transaction[key]

        return self._content[key]

    def __getitem__(self, key):
        return self._data_converters.strip_version(key, self.get_with_version(key))

    def __setitem__(self, key, value):
        transaction = {key: value}
        self.put(transaction)

    def __delitem__(self, key):
        transaction = {key: None}
        self.put(transaction)

    def __iter__(self):
        included_keys = set()
        excluded_keys = set()
        for _, transaction in reversed(self._pending_validation):
            for key, value in transaction.items():
                if value != None:
                    included_keys.add(key)
                else:
                    excluded_keys.add(key)
        for key in self._content.keys():
            if key not in excluded_keys:
                included_keys.add(key)
        return (key for key in included_keys)

    def __len__(self):
        return len(self._content)


