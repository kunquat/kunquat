# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import deque

from kunquat.kunquat.limits import *


class Note():

    def __init__(self, channel):
        self._controller = None
        self._alive = True
        self._channel = channel

    def set_controller(self, controller):
        self._controller = controller

    def kill(self):
        self._alive = False

    def is_alive(self):
        return self._alive

    def set_rest(self):
        if self.is_alive():
            self._controller.set_rest(self._channel)
            self.kill()

    def get_channel(self):
        return self._channel


class NoteChannelMapper():

    def __init__(self):
        self._controller = None
        self._tracked_notes = {}

    def set_controller(self, controller):
        self._controller = controller

    def get_tracked_note(self, channel, force_ch):
        if force_ch:
            selected_channel = channel
        else:
            selected_channel = self._get_free_channel(channel)

        old_note = self._tracked_notes.pop(selected_channel, None)
        if old_note:
            old_note.kill()

        new_note = Note(selected_channel)
        new_note.set_controller(self._controller)
        self._tracked_notes[selected_channel] = new_note
        return new_note

    def _get_free_channel(self, preferred):
        channels = deque(range(CHANNELS_MAX))
        channels.rotate(-preferred) # Start searching from preferred

        for ch in channels:
            if ch not in self._tracked_notes or not self._tracked_notes[ch].is_alive():
                return ch
        else:
            return preferred


