# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import deque
from itertools import count


class Session():

    def __init__(self):
        self._output_speed = 0
        self._render_speed = 0
        self._render_load = 0
        self._ui_lag = 0
        self._progress_position = 1
        self._progress_steps = 1
        self._audio_levels = (0, 0)
        self._channel_selected_control_id = dict()
        self._channel_active_control_id = dict()
        self._channel_active_note = dict()
        self._control_active_notes = dict()
        self._octave_id = 0
        self._visible = set()
        self._event_log = deque([], 1024)
        self._event_index = count()
        self._selected_location = None
        self._active_notes = {}
        self._sheet_zoom = 0
        self._sheet_zoom_min = 0
        self._sheet_zoom_max = 0
        self._sheet_column_width = 0
        self._sheet_column_width_min = 0
        self._sheet_column_width_max = 0
        self._last_column = None

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

    def get_selected_control_id_by_channel(self, channel):
        return self._channel_selected_control_id[channel]

    def set_selected_control_id(self, channel, control):
        control_id = 'control_{0:02x}'.format(control)
        self._channel_selected_control_id[channel] = control_id

    def get_active_control_id_by_channel(self, channel):
        return self._channel_active_control_id[channel]

    def get_active_note_by_channel(self, channel):
        return self._channel_active_note[channel]

    def get_active_notes_by_control_id(self, control_id):
        try:
            notes = self._control_active_notes[control_id]
        except KeyError:
            notes = dict()
        return notes

    def set_active_note(self, channel, pitch):
        if pitch == None:
            if channel in self._channel_active_note:
                del self._channel_active_note[channel]
            control_id = self.get_active_control_id_by_channel(channel)
            if control_id in self._control_active_notes:
               notes = self._control_active_notes[control_id]
               if channel in notes:
                   del notes[channel]
        else:
            self._channel_active_note[channel] = pitch
            control_id = self.get_selected_control_id_by_channel(channel)
            if not control_id in self._control_active_notes:
                self._control_active_notes[control_id] = dict()
            notes = self._control_active_notes[control_id]
            notes[channel] = pitch
            self._channel_active_control_id[channel] = control_id

    def set_octave_id(self, octave_id):
        self._octave_id = octave_id

    def get_octave_id(self):
        return self._octave_id

    def show_ui(self, ui_id):
        assert ui_id
        self._visible.add(ui_id)

    def hide_ui(self, ui_id):
        assert ui_id
        self._visible -= set([ui_id])

    def hide_all(self):
        self._visible = set()

    def get_visible(self):
        return self._visible

    def log_event(self, channel, event_type, event_value, context):
        self._event_log.appendleft(
                (self._event_index.next(),
                    channel, event_type, event_value, context))

    def get_event_log(self):
        return list(self._event_log)

    def get_keymap_name(self):
        return '12tet' # TODO: make this configurable

    def get_notation_name(self):
        return '12tet' # TODO: make this configurable

    def set_selected_location(self, trigger_position):
        self._selected_location = trigger_position

    def get_selected_location(self):
        return self._selected_location

    def activate_key_with_note(self, row, index, note):
        assert not self.is_key_active(row, index)
        key = (row, index)
        self._active_notes[key] = note

    def deactivate_key(self, row, index):
        key = (row, index)
        del self._active_notes[key]

    def is_key_active(self, row, index):
        key = (row, index)
        return key in self._active_notes

    def get_active_note(self, row, index):
        key = (row, index)
        return self._active_notes[key]

    def set_sheet_zoom(self, zoom):
        self._sheet_zoom = min(max(self._sheet_zoom_min, zoom), self._sheet_zoom_max)

    def set_sheet_zoom_range(self, minimum, maximum):
        self._sheet_zoom_min = minimum
        self._sheet_zoom_max = max(minimum, maximum)
        self.set_sheet_zoom(self._sheet_zoom)

    def get_sheet_zoom(self):
        return self._sheet_zoom

    def get_sheet_zoom_range(self):
        return (self._sheet_zoom_min, self._sheet_zoom_max)

    def set_sheet_column_width(self, width):
        self._sheet_column_width = min(max(
            self._sheet_column_width_min, width), self._sheet_column_width_max)

    def set_sheet_column_width_range(self, minimum, maximum):
        self._sheet_column_width_min = minimum
        self._sheet_column_width_max = max(minimum, maximum)
        self.set_sheet_column_width(self._sheet_column_width)

    def get_sheet_column_width(self):
        return self._sheet_column_width

    def get_sheet_column_width_range(self):
        return (self._sheet_column_width_min, self._sheet_column_width_max)

    def set_last_column(self, column):
        self._last_column = column

    def get_last_column(self):
        return self._last_column


