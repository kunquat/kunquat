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
        self._instruments = {}

    def get_instrument(self, instrument_number):
        if not instrument_number in self._instruments:
            new_instrument = Instrument()
            new_instrument.set_instrument_number(instrument_number)
            self._instruments[instrument_number] = new_instrument
        return self._instruments[instrument_number]

