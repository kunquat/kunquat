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
        self._channel_selected_instrument = dict()
        self._channel_active_instrument = dict()
        self._channel_active_note = dict()
        self._instrument_active_notes = dict()

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

    def get_selected_instrument_by_channel(self, channel):
        return self._channel_selected_instrument[channel]

    def set_selected_instrument(self, channel, instrument):
        instrument_id = 'ins_{0:02x}'.format(instrument)
        self._channel_selected_instrument[channel] = instrument_id

    def get_active_instrument_by_channel(self, channel):
        return self._channel_active_instrument[channel]

    def get_active_note_by_channel(self, channel):
        return self._channel_active_note[channel]

    def get_active_notes_by_instrument(self, instrument):
        try:
            notes = self._instrument_active_notes[instrument]
        except KeyError:
            notes = dict()
        return notes

    def set_active_note(self, channel, pitch):
        if pitch == None:
            if channel in self._channel_active_note:
                del self._channel_active_note[channel]
            instrument = self.get_active_instrument_by_channel(channel)
            if instrument in self._instrument_active_notes:
               notes = self._instrument_active_notes[instrument]
               if channel in notes:
                   del notes[channel]
        else:
            self._channel_active_note[channel] = pitch
            instrument = self.get_selected_instrument_by_channel(channel)
            if not instrument in self._instrument_active_notes:
                self._instrument_active_notes[instrument] = dict()
            notes = self._instrument_active_notes[instrument]
            notes[channel] = pitch
            self._channel_active_instrument[channel] = instrument

