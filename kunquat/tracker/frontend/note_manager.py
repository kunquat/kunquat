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

class NoteManager(Updater):

    def __init__(self):
        super(NoteManager, self).__init__()
        self._backend = None
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
        pass

    def update_active_note(self, channel_number, pitch):
        self._active_notes[channel_number] = pitch
        self._signal_update()

