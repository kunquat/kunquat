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

from instrument import Instrument


class Module():

    def __init__(self):
        self._updater = None
        self._store = None
        self._controller = None
        self._backend = None
        self._instruments = {}

    def set_store(self, store):
        self._store = store

    def set_controller(self, controller):
        self._controller = controller

    def set_backend(self, backend):
        self._backend = backend

    def set_updater(self, updater):
        self._updater = updater

    def get_instrument(self, instrument_id):
        instrument = Instrument(instrument_id)
        instrument.set_store(self._store)
        instrument.set_controller(self._controller)
        return instrument

    def update_instrument(self, instrument_number, instrument):
        instrument.set_updater(self._updater)
        self._instruments[instrument_number] = instrument
        self._updater.signal_update()

    def get_instrument_ids(self):
        instrument_ids = set()
        for key in self._store.keys():
            if key.startswith('ins_'):
                instrument_id = key.split('/')[0]
                instrument_ids.add(instrument_id)
        return instrument_ids

    def get_instruments(self, validate=True):
        instrument_ids = self.get_instrument_ids()
        all_instruments = [self.get_instrument(i) for i in instrument_ids]
        #all_instruments = self._instruments.values()
        #if validate:
        #    valid = [i for i in all_instruments if i.get_existence()]
        #    return [] #valid
        return all_instruments

