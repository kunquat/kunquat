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


class Session():

    def __init__(self):
        self._output_speed = 0
        self._render_speed = 0
        self._render_load = 0
        self._ui_lag = 0
        self._progress_position = 1
        self._progress_steps = 1
        self._audio_levels = (0, 0)
        self._channel_selected_slot_id = dict()
        self._channel_active_slot_id = dict()
        self._channel_active_note = dict()
        self._slot_active_notes = dict()

    def get_output_speed(self):
        return self._output_speed

    def set_output_speed(self, output_speed):
        self._output_speed = output_speed

    def get_render_speed(self):
        return self._render_speed

    def set_render_speed(self, render_speed):
        self._render_speed = render_speed

    def get_render_load(self):
        return self._render_load

    def set_render_load(self, render_load):
        self._render_load = render_load

    def get_ui_lag(self):
        return self._ui_lag

    def set_ui_lag(self, ui_lag):
        self._ui_lag = ui_lag

    def get_progress_position(self):
        return self._progress_position

    def set_progress_position(self, progress_position):
        self._progress_position = progress_position

    def get_progress_steps(self):
        return self._progress_steps

    def set_progress_steps(self, progress_steps):
        self._progress_steps = progress_steps

    def get_audio_levels(self):
        return self._audio_levels

    def set_audio_levels(self, audio_levels):
        self._audio_levels = audio_levels

    def get_selected_slot_id_by_channel(self, channel):
        return self._channel_selected_slot_id[channel]

    def set_selected_slot_id(self, channel, slot):
        slot_id = 'slot_{0:02x}'.format(slot)
        self._channel_selected_slot_id[channel] = slot_id

    def get_active_slot_id_by_channel(self, channel):
        return self._channel_active_slot_id[channel]

    def get_active_note_by_channel(self, channel):
        return self._channel_active_note[channel]

    def get_active_notes_by_slot_id(self, slot_id):
        try:
            notes = self._slot_active_notes[slot_id]
        except KeyError:
            notes = dict()
        return notes

    def set_active_note(self, channel, pitch):
        if pitch == None:
            if channel in self._channel_active_note:
                del self._channel_active_note[channel]
            slot_id = self.get_active_slot_id_by_channel(channel)
            if slot_id in self._slot_active_notes:
               notes = self._slot_active_notes[slot_id]
               if channel in notes:
                   del notes[channel]
        else:
            self._channel_active_note[channel] = pitch
            slot_id = self.get_selected_slot_id_by_channel(channel)
            if not slot_id in self._slot_active_notes:
                self._slot_active_notes[slot_id] = dict()
            notes = self._slot_active_notes[slot_id]
            notes[channel] = pitch
            self._channel_active_slot_id[channel] = slot_id

