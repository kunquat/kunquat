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

from updater import Updater


class Instrument(Updater):

    def __init__(self):
        super(Instrument, self).__init__()
        self._backend = None
        self._instrument_number = None
        self._active_notes = {}

    def set_backend(self, backend):
        self._backend = backend

    def get_active_notes(self):
        return self._active_notes.items()

    def get_active_note(self, channel_number):
        if channel_number not in self._active_note:
            return None
        return self._active_notes[channel_number]

    def set_active_note(self, channel_number, pitch):
        self._backend.set_active_note(channel_number, self._instrument_number, pitch)

    def update_active_note(self, channel_number, pitch):
        if pitch != None:
            self._active_notes[channel_number] = pitch
        elif channel_number in self._active_notes:
            del self._active_notes[channel_number]
        self._signal_update()

    def set_instrument_number(self, instrument_number):
        self._instrument_number = instrument_number
      
    def get_instrument_number(self):
        return self._instrument_number

