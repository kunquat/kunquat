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

from note_manager import NoteManager


class Instrument():

    def __init__(self):
        self._instrument_number = None
        self._note_manager = None

    def set_instrument_number(self, instrument_number):
        self._instrument_number = instrument_number
      
    def get_instrument_number(self):
        return self._instrument_number

    def get_note_manager(self):
        if not self._note_manager:
            self._note_manager = NoteManager()
        return self._note_manager

