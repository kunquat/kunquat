# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013
#          Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#


class Instrument():

    def __init__(self, instrument_id):
        self._instrument_id = instrument_id
        self._store = None
        self._backend = None
        self._instrument_number = None
        self._existence = None
        self._updater = None
        self._active_notes = {}

    def set_store(self, store):
        self._store = store

    def set_backend(self, backend):
        self._backend = backend

    def set_updater(self, updater):
        self._updater = updater

    def get_active_notes(self):
        return self._active_notes.items()

    def get_active_note(self, channel_number):
        if channel_number not in self._active_note:
            return None
        return self._active_notes[channel_number]

    def set_active_note(self, channel_number, pitch):
        self._backend.set_active_note(channel_number, self._instrument_number, pitch)

    def get_id(self):
        return self._instrument_id

    def get_existence(self):
        key = '%s/p_manigest.json' % self._instrument_id
        manifest = self._store[key]
        if type(manifest) == type({}):
            return True
        return False

    def get_name(self):
        key = '%s/m_name.json' % self._instrument_id
        name = self._store[key]
        return name

