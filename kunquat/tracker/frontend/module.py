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


from updater import Updater

class Module(Updater):

    def __init__(self):
        super(Module, self).__init__()
        self._InstrumentClass = None
        self._instruments = {}

    def set_instrument_class(self, InstrumentClass):
        self._InstrumentClass = InstrumentClass

    def get_instrument(self, instrument_number):
        if not instrument_number in self._instruments:
            new_instrument = self._InstrumentClass()
            new_instrument.set_instrument_number(instrument_number)
            self.register_child(new_instrument)
            self._instruments[instrument_number] = new_instrument
        return self._instruments[instrument_number]

