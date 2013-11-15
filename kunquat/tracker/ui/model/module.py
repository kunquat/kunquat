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


class Module():

    def __init__(self):
        self._backend = None
        self._instruments = {}
        self._updater = None

    def set_backend(self, backend):
        self._backend = backend

    def set_updater(self, updater):
        self._updater = updater

    def get_instrument(self, instrument_number):
        return self._instruments[instrument_number]

    def update_instrument(self, instrument_number, instrument):
        instrument.set_updater(self._updater)
        self._instruments[instrument_number] = instrument
        self._updater.signal_update()

    def get_instruments(self, validate=True):
        all_instruments = self._instruments.values()
        if validate:
            valid = [i for i in all_instruments if i.get_existence()]
            return valid
        return all_instruments

