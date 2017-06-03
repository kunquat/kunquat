# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2013-2017
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
        self._collected_render_loads = []
        self._render_load_averages = deque([], 3600)
        self._render_load_peaks = deque([], 3600)
        self._ui_load = 0
        self._ui_load_averages = deque([], 3600)
        self._ui_load_peaks = deque([], 3600)
        self._progress_description = None
        self._progress_position = 0
        self._audio_levels = (0, 0)
        self._max_audio_levels = [0, 0]
        self._infinite_mode = False
        self._playback_track = None
        self._selected_control_id = 0
        self._is_hit_keymap_active = False
        self._selected_notation_id = (True, '12tetsharp')
        self._notation_editor_selected_notation_id = None
        self._notation_editor_selected_octave_id = None
        self._notation_editor_selected_note_index = None
        self._notation_editor_selected_key_index = None
        self._notation_editor_selected_template_note = None
        self._notation_editor_selected_tuning_table_id = None
        self._tuning_table_selected_notes = {}
        self._control_id_override = None
        self._enabled_test_processors = set()
        self._test_processors = {}
        self._test_processor_params = {}
        # TODO: get default control ids from libkunquat?
        self._channel_selected_control_id = defaultdict(lambda: 0)
        self._channel_active_control_id = defaultdict(lambda: 0)
        self._channel_active_note = {}
        self._control_active_notes = {}
        self._channel_active_ch_expression = {}
        self._channel_default_ch_expression = {}
        self._octave_id = None
        self._visible = set()
        self._event_log = deque([], 1024)
        self._event_index = count()
        self._selected_location = None
        self._selected_area_start = None
        self._selected_area_stop = None
        self._chord_mode = False
        self._chord_start = None
        self._chord_notes = set()
        self._active_notes = {}
        self._sheet_past = []
        self._sheet_cur_step = None
        self._sheet_future = []
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
        self._is_playback_active = False
        self._record_mode_enabled = False
        self._is_grid_enabled = True
        self._selected_grid_pattern_id = None
        self._selected_grid_pattern_line = None
        self._gp_subdiv_part_count = 2
        self._gp_subdiv_line_style = 1
        self._gp_subdiv_warp = 0.5
        self._gp_zoom = 0
        self._gp_zoom_min = 0
        self._gp_zoom_max = 0
        self._default_grid_pattern_id = None
        self._pending_playback_cursor_track = 0
        self._pending_playback_cursor_system = 0
        self._playback_cursor_position = (0, 0, [0, 0])
        self._playback_pattern = None
        self._orderlist_selection = None
        self._track_selection = 0
        self._expanded_au_vars = {}
        self._module_path = None
        self._is_saving = False
        self._active_var_names = {}
        self._runtime_env = {}
        self._selected_binding_index = None
        self._module_load_error_info = None
        self._au_import_info = None
        self._au_import_error_info = None
        self._au_export_info = None
        self._au_conns_edit_mode = {}
        self._au_conns_hit_index = {}
        self._au_conns_expr_name = {}
        self._au_expressions = {}
        self._au_test_forces = {}
        self._au_test_expressions = {}
        self._au_test_params_enabled = {}
        self._edit_selected_hits = {}
        self._selected_sample_ids = {}
        self._selected_sample_note_map_points = {}
        self._selected_sample_hit_info = {}
        self._selected_sample_hit_map_force = {}

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
        self._collected_render_loads.append(render_load)

    def _update_render_load_history(self):
        if self._collected_render_loads:
            avg = (sum(self._collected_render_loads) /
                    float(len(self._collected_render_loads)))
            self._render_load_averages.append(avg)
            self._render_load_peaks.append(max(self._collected_render_loads))
            self._collected_render_loads = []

    def get_render_load_averages(self):
        self._update_render_load_history()
        return list(self._render_load_averages)

    def get_render_load_peaks(self):
        self._update_render_load_history()
        return list(self._render_load_peaks)

    def get_ui_load(self):
        return self._ui_load

    def set_ui_load(self, ui_load):
        self._ui_load = ui_load

    def get_ui_load_averages(self):
        return list(self._ui_load_averages)

    def add_ui_load_average(self, load_avg):
        self._ui_load_averages.append(load_avg)

    def get_ui_load_peaks(self):
        return list(self._ui_load_peaks)

    def add_ui_load_peak(self, load_peak):
        self._ui_load_peaks.append(load_peak)

    def get_progress_description(self):
        return self._progress_description

    def set_progress_description(self, desc):
        self._progress_description = desc

    def get_progress_position(self):
        return self._progress_position

    def set_progress_position(self, progress_position):
        self._progress_position = progress_position

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

    def set_playback_track(self, track_num):
        self._playback_track = track_num

    def get_playback_track(self):
        return self._playback_track

    def get_selected_control_id(self):
        return self._selected_control_id

    def set_selected_control_id(self, control_id):
        self._selected_control_id = control_id

    def is_hit_keymap_active(self):
        return self._is_hit_keymap_active

    def set_hit_keymap_active(self, active):
        self._is_hit_keymap_active = active

    def get_selected_notation_id(self):
        return self._selected_notation_id

    def set_selected_notation_id(self, notation_id):
        self._selected_notation_id = notation_id

    def get_notation_editor_selected_notation_id(self):
        return self._notation_editor_selected_notation_id

    def set_notation_editor_selected_notation_id(self, notation_id):
        self._notation_editor_selected_notation_id = notation_id

    def get_notation_editor_selected_octave_id(self):
        return self._notation_editor_selected_octave_id

    def set_notation_editor_selected_octave_id(self, octave_id):
        self._notation_editor_selected_octave_id = octave_id

    def get_notation_editor_selected_note_index(self):
        return self._notation_editor_selected_note_index

    def set_notation_editor_selected_note_index(self, note_index):
        self._notation_editor_selected_note_index = note_index

    def get_notation_editor_selected_key_index(self):
        return self._notation_editor_selected_key_index

    def set_notation_editor_selected_key_index(self, key_index):
        self._notation_editor_selected_key_index = key_index

    def get_notation_editor_selected_template_note(self):
        return self._notation_editor_selected_template_note

    def set_notation_editor_selected_template_note(self, coords):
        self._notation_editor_selected_template_note = coords

    def get_notation_editor_selected_tuning_table_id(self):
        return self._notation_editor_selected_tuning_table_id

    def set_notation_editor_selected_tuning_table_id(self, table_id):
        self._notation_editor_selected_tuning_table_id = table_id

    def get_tuning_table_selected_note(self, table_id):
        return self._tuning_table_selected_notes.get(table_id, None)

    def set_tuning_table_selected_note(self, table_id, index):
        self._tuning_table_selected_notes[table_id] = index

    def get_control_id_override(self):
        return self._control_id_override

    def set_control_id_override(self, control_id):
        self._control_id_override = control_id

    def is_processor_testing_enabled(self, proc_id):
        return proc_id in self._enabled_test_processors

    def set_processor_testing_enabled(self, proc_id, enabled):
        if enabled:
            self._enabled_test_processors.add(proc_id)
        else:
            self._enabled_test_processors.discard(proc_id)

    def get_test_processor(self, control_id):
        return self._test_processors.get(control_id, None)

    def set_test_processor(self, control_id, proc_id):
        self._test_processors[control_id] = proc_id

    def get_test_processor_param(self, proc_id):
        return self._test_processor_params.get(proc_id, None)

    def set_test_processor_param(self, proc_id, param):
        self._test_processor_params[proc_id] = param

    def get_selected_control_id_by_channel(self, channel):
        return self._channel_selected_control_id[channel]

    def set_selected_control_id_by_channel(self, channel, control):
        control_id = 'control_{0:02x}'.format(control)
        self._channel_selected_control_id[channel] = control_id

    def get_active_control_id_by_channel(self, channel):
        return self._channel_active_control_id[channel]

    def get_active_note_by_channel(self, channel):
        event_type, pitch = self._channel_active_note[channel]
        if event_type == 'n+':
            return pitch
        return None

    def get_active_hit_by_channel(self, channel):
        event_type, hit = self._channel_active_note[channel]
        if event_type == 'h':
            return hit
        return None

    def get_active_notes_by_control_id(self, control_id):
        notes = self._control_active_notes.get(control_id, {})
        ret = {}
        for (k, v) in notes.items():
            event_type, pitch = v
            if event_type == 'n+':
                ret[k] = pitch
        return ret

    def get_active_hits_by_control_id(self, control_id):
        notes = self._control_active_notes.get(control_id, {})
        hits = {}
        for (k, v) in notes.items():
            event_type, hit = v
            if event_type == 'h':
                hits[k] = hit
        return hits

    def set_active_note(self, channel, event_type, param):
        if param == None:
            if channel in self._channel_active_note:
                del self._channel_active_note[channel]
            control_id = self.get_active_control_id_by_channel(channel)
            if control_id in self._control_active_notes:
                notes = self._control_active_notes[control_id]
                if channel in notes:
                    del notes[channel]
        else:
            self._channel_active_note[channel] = (event_type, param)
            control_id = self.get_selected_control_id_by_channel(channel)
            if not control_id in self._control_active_notes:
                self._control_active_notes[control_id] = dict()
            notes = self._control_active_notes[control_id]
            notes[channel] = (event_type, param)
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
        if context != 'tfire':
            self._event_log.appendleft(
                    (next(self._event_index),
                        channel, event_type, event_value, context))

    def get_event_log(self):
        return list(self._event_log)

    def set_selected_location(self, trigger_position):
        self._selected_location = trigger_position

    def get_selected_location(self):
        return self._selected_location

    def set_selected_area_start(self, trigger_position):
        self._selected_area_start = trigger_position

    def get_selected_area_start(self):
        return self._selected_area_start

    def set_selected_area_stop(self, trigger_position):
        self._selected_area_stop = trigger_position

    def get_selected_area_stop(self):
        return self._selected_area_stop

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

    def set_chord_note(self, event_type, param, is_down):
        if is_down:
            self._chord_notes.add((event_type, param))
        else:
            self._chord_notes.discard((event_type, param))

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

    def get_sheet_past(self):
        return self._sheet_past

    def get_sheet_cur_step(self):
        return self._sheet_cur_step

    def set_sheet_cur_step(self, cur_step):
        self._sheet_cur_step = cur_step

    def get_sheet_future(self):
        return self._sheet_future

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

    def set_playback_active(self, active):
        self._is_playback_active = active

    def is_playback_active(self):
        return self._is_playback_active

    def set_record_mode(self, enabled):
        self._record_mode_enabled = enabled

    def get_record_mode(self):
        return self._record_mode_enabled

    def set_pending_playback_cursor_track(self, track):
        self._pending_playback_cursor_track = track

    def set_pending_playback_cursor_system(self, system):
        self._pending_playback_cursor_system = system

    def set_playback_pattern(self, piref):
        self._playback_pattern = piref

    def get_playback_pattern(self):
        return self._playback_pattern

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

    def set_grid_pattern_subdiv_part_count(self, count):
        self._gp_subdiv_part_count = count

    def get_grid_pattern_subdiv_part_count(self):
        return self._gp_subdiv_part_count

    def set_grid_pattern_subdiv_line_style(self, style):
        self._gp_subdiv_line_style = style

    def get_grid_pattern_subdiv_line_style(self):
        return self._gp_subdiv_line_style

    def set_grid_pattern_subdiv_warp(self, warp):
        self._gp_subdiv_warp = warp

    def get_grid_pattern_subdiv_warp(self):
        return self._gp_subdiv_warp

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

    def set_default_grid_pattern_id(self, gp_id):
        self._default_grid_pattern_id = gp_id

    def get_default_grid_pattern_id(self):
        return self._default_grid_pattern_id

    def set_au_var_expanded(self, au_id, var_name, expanded):
        au_vars = self._expanded_au_vars.get(au_id, set())
        if expanded:
            au_vars.add(var_name)
        else:
            au_vars.discard(var_name)
        self._expanded_au_vars[au_id] = au_vars

    def is_au_var_expanded(self, au_id, var_name):
        return (var_name in self._expanded_au_vars.get(au_id, set()))

    def set_module_path(self, path):
        self._module_path = path

    def get_module_path(self):
        return self._module_path

    def set_module_load_error_info(self, path, msg):
        self._module_load_error_info = (path, msg)

    def get_module_load_error_info(self):
        return self._module_load_error_info

    def set_saving(self, enabled):
        self._is_saving = enabled

    def is_saving(self):
        return self._is_saving

    def set_active_var_name(self, ch, var_name):
        self._active_var_names[ch] = var_name

    def get_active_var_name(self, ch):
        return self._active_var_names.get(ch)

    def set_active_var_value(self, ch, var_value):
        var_name = self.get_active_var_name(ch)
        if var_name:
            self._runtime_env[var_name] = var_value

    def get_runtime_var_value(self, var_name):
        return self._runtime_env.get(var_name)

    def reset_runtime_env(self):
        self._active_var_names.clear()
        self._runtime_env.clear()

    def set_selected_binding_index(self, index):
        self._selected_binding_index = index

    def get_selected_binding_index(self):
        return self._selected_binding_index

    def set_au_import_info(self, info):
        self._au_import_info = info

    def get_au_import_info(self):
        return self._au_import_info

    def is_importing_audio_unit(self):
        return self._au_import_info != None

    def set_au_import_error_info(self, path, error):
        self._au_import_error_info = (path, error)

    def get_reset_au_import_error_info(self):
        info = self._au_import_error_info
        self._au_import_error_info = None
        return info

    def set_au_export_info(self, info):
        self._au_export_info = info

    def get_au_export_info(self):
        return self._au_export_info

    def set_au_connections_edit_mode(self, au_id, mode):
        self._au_conns_edit_mode[au_id] = mode

    def get_au_connections_edit_mode(self, au_id):
        return self._au_conns_edit_mode.get(au_id, None)

    def set_au_connections_hit_index(self, au_id, hit_index):
        self._au_conns_hit_index[au_id] = hit_index

    def get_au_connections_hit_index(self, au_id):
        return self._au_conns_hit_index.get(au_id, None)

    def set_edit_selected_hit_info(self, au_id, hit_base, hit_offset):
        self._edit_selected_hits[au_id] = (hit_base, hit_offset)

    def get_edit_selected_hit_info(self, au_id):
        return self._edit_selected_hits.get(au_id, (0, 0))

    def set_selected_expression(self, au_id, name):
        self._au_expressions[au_id] = name

    def get_selected_expression(self, au_id):
        return self._au_expressions.get(au_id, None)

    def set_au_connections_expr_name(self, au_id, expr_name):
        self._au_conns_expr_name[au_id] = expr_name

    def get_au_connections_expr_name(self, au_id):
        return self._au_conns_expr_name.get(au_id, None)

    def set_active_ch_expression(self, ch, expr_name):
        self._channel_active_ch_expression[ch] = expr_name

    def set_default_ch_expression(self, ch, expr_name):
        self._channel_default_ch_expression[ch] = expr_name

    def get_active_ch_expression(self, ch):
        return self._channel_active_ch_expression.get(ch,
                self._channel_default_ch_expression.get(ch, ''))

    def reset_active_ch_expressions(self):
        self._channel_active_ch_expression = {}

    def set_au_test_force(self, au_id, force):
        self._au_test_forces[au_id] = force

    def get_au_test_force(self, au_id):
        return self._au_test_forces.get(au_id, 0)

    def set_au_test_expression(self, au_id, index, expr_name):
        self._au_test_expressions[(au_id, index)] = expr_name

    def get_au_test_expression(self, au_id, index):
        return self._au_test_expressions.get((au_id, index), '')

    def set_au_test_params_enabled(self, au_id, enabled):
        self._au_test_params_enabled[au_id] = enabled

    def are_au_test_params_enabled(self, au_id):
        return self._au_test_params_enabled.get(au_id)

    def set_selected_sample_id(self, proc_id, sample_id):
        self._selected_sample_ids[proc_id] = sample_id

    def get_selected_sample_id(self, proc_id):
        return self._selected_sample_ids.get(proc_id, None)

    def set_selected_sample_note_map_point(self, proc_id, point):
        self._selected_sample_note_map_points[proc_id] = point

    def get_selected_sample_note_map_point(self, proc_id):
        return self._selected_sample_note_map_points.get(proc_id, None)

    def set_selected_sample_hit_info(self, proc_id, hit_info):
        self._selected_sample_hit_info[proc_id] = hit_info

    def get_selected_sample_hit_info(self, proc_id):
        return self._selected_sample_hit_info.get(proc_id, (0, 0))

    def set_selected_sample_hit_map_force(self, proc_id, force):
        self._selected_sample_hit_map_force[proc_id] = force

    def get_selected_sample_hit_map_force(self, proc_id):
        return self._selected_sample_hit_map_force.get(proc_id, None)


