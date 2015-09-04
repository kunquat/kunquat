# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2013-2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import defaultdict, deque
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
        self._max_audio_levels = [0, 0]
        self._infinite_mode = False
        self._selected_control_id = 0
        self._selected_keymap_id = None
        self._selected_notation_id = None
        self._control_id_override = None
        # TODO: get default control ids from libkunquat?
        self._channel_selected_control_id = defaultdict(lambda: 0)
        self._channel_active_control_id = defaultdict(lambda: 0)
        self._channel_active_note = dict()
        self._control_active_notes = dict()
        self._octave_id = None
        self._visible = set()
        self._event_log = deque([], 1024)
        self._event_index = count()
        self._selected_location = None
        self._chord_mode = False
        self._chord_start = None
        self._chord_notes = set()
        self._active_notes = {}
        self._sheet_zoom = 0
        self._sheet_zoom_min = 0
        self._sheet_zoom_max = 0
        self._sheet_column_width = 0
        self._sheet_column_width_min = 0
        self._sheet_column_width_max = 0
        self._last_column = None
        self._edit_mode_enabled = False
        self._typewriter_connected = False
        self._replace_mode_enabled = False
        self._record_mode_enabled = False
        self._is_grid_enabled = True
        self._selected_grid_pattern_id = None
        self._selected_grid_pattern_line = None
        self._gp_zoom = 0
        self._gp_zoom_min = 0
        self._gp_zoom_max = 0
        self._pending_playback_cursor_track = 0
        self._pending_playback_cursor_system = 0
        self._playback_cursor_position = (0, 0, [0, 0])
        self._orderlist_selection = None
        self._track_selection = 0
        self._module_path = None
        self._is_saving = False
        self._active_var_names = defaultdict(lambda: {})
        self._runtime_env = {}

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
        for ch, level in enumerate(audio_levels):
            self._max_audio_levels[ch] = max(self._max_audio_levels[ch], level)

    def get_max_audio_levels(self):
        return self._max_audio_levels

    def reset_max_audio_levels(self):
        self._max_audio_levels = [0, 0]

    def set_infinite_mode(self, enabled):
        self._infinite_mode = enabled

    def get_infinite_mode(self):
        return self._infinite_mode

    def get_selected_control_id(self):
        return self._selected_control_id

    def set_selected_control_id(self, control_id):
        self._selected_control_id = control_id

    def get_selected_keymap_id(self):
        return self._selected_keymap_id

    def set_selected_keymap_id(self, keymap_id):
        self._selected_keymap_id = keymap_id

    def get_selected_notation_id(self):
        return self._selected_notation_id

    def set_selected_notation_id(self, notation_id):
        self._selected_notation_id = notation_id

    def get_control_id_override(self):
        return self._control_id_override

    def set_control_id_override(self, control_id):
        self._control_id_override = control_id

    def get_selected_control_id_by_channel(self, channel):
        return self._channel_selected_control_id[channel]

    def set_selected_control_id_by_channel(self, channel, control):
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

    def set_selected_location(self, trigger_position):
        self._selected_location = trigger_position

    def get_selected_location(self):
        return self._selected_location

    def set_chord_mode(self, enabled):
        self._chord_mode = enabled
        if not enabled:
            self._chord_notes.clear()

    def get_chord_mode(self):
        return self._chord_mode

    def set_chord_start(self, location):
        self._chord_start = location

    def get_chord_start(self):
        return self._chord_start

    def set_chord_note(self, note, is_down):
        if is_down:
            self._chord_notes.add(note)
        else:
            self._chord_notes.discard(note)

    def are_chord_notes_down(self):
        return len(self._chord_notes) != 0

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

    def set_edit_mode(self, enabled):
        self._edit_mode_enabled = enabled

    def get_edit_mode(self):
        return self._edit_mode_enabled

    def set_typewriter_connected(self, connected):
        self._typewriter_connected = connected

    def get_typewriter_connected(self):
        return self._typewriter_connected

    def set_replace_mode(self, enabled):
        self._replace_mode_enabled = enabled

    def get_replace_mode(self):
        return self._replace_mode_enabled

    def set_record_mode(self, enabled):
        self._record_mode_enabled = enabled

    def get_record_mode(self):
        return self._record_mode_enabled

    def set_pending_playback_cursor_track(self, track):
        self._pending_playback_cursor_track = track

    def set_pending_playback_cursor_system(self, system):
        self._pending_playback_cursor_system = system

    def set_playback_cursor(self, row):
        track = self._pending_playback_cursor_track
        system = self._pending_playback_cursor_system
        self._playback_cursor_position = (track, system, row)

    def get_playback_cursor_position(self):
        return self._playback_cursor_position

    def set_orderlist_selection(self, selection):
        self._orderlist_selection = selection

    def get_orderlist_selection(self):
        return self._orderlist_selection

    def set_selected_track_num(self, track_num):
        self._track_selection = track_num

    def get_selected_track_num(self):
        return self._track_selection

    def set_grid_enabled(self, enabled):
        self._is_grid_enabled = enabled

    def is_grid_enabled(self):
        return self._is_grid_enabled

    def select_grid_pattern(self, gp_id):
        self._selected_grid_pattern_id = gp_id

    def get_selected_grid_pattern_id(self):
        return self._selected_grid_pattern_id

    def select_grid_pattern_line(self, line_ts):
        self._selected_grid_pattern_line = line_ts

    def get_selected_grid_pattern_line(self):
        return self._selected_grid_pattern_line

    def set_grid_pattern_zoom(self, zoom):
        self._gp_zoom = min(max(self._gp_zoom_min, zoom), self._gp_zoom_max)

    def set_grid_pattern_zoom_range(self, minimum, maximum):
        self._gp_zoom_min = minimum
        self._gp_zoom_max = max(minimum, maximum)
        self.set_grid_pattern_zoom(self._gp_zoom)

    def get_grid_pattern_zoom(self):
        return self._gp_zoom

    def get_grid_pattern_zoom_range(self):
        return (self._gp_zoom_min, self._gp_zoom_max)

    def set_module_path(self, path):
        self._module_path = path

    def get_module_path(self):
        return self._module_path

    def set_saving(self, enabled):
        self._is_saving = enabled

    def is_saving(self):
        return self._is_saving

    def set_active_var_name(self, ch, var_type, var_name):
        self._active_var_names[ch][var_type] = var_name

    def get_active_var_name(self, ch, var_type):
        return self._active_var_names[ch].get(var_type)

    def set_active_var_value(self, ch, var_type, var_value):
        var_name = self.get_active_var_name(ch, var_type)
        if var_name:
            self._runtime_env[(var_type, var_name)] = var_value

    def get_runtime_var_value(self, var_type, var_name):
        return self._runtime_env.get((var_type, var_name))

    def reset_runtime_env(self):
        self._active_var_names.clear()
        self._runtime_env.clear()


