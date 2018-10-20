# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from copy import deepcopy
import json
import os
import os.path
import string
import sys


class _Entry():

    def __init__(self, validator, value):
        assert validator(value)
        self._value = value
        self._validator = validator

    def __repr__(self):
        return repr(self.get())

    def accepts(self, value):
        return self._validator(value)

    def get_modified(self, value):
        return _Entry(self._validator, value)

    def set(self, value):
        if not self.accepts(value):
            raise ValueError('Invalid value for config entry')
        self._value = value

    def get(self):
        return deepcopy(self._value)


class _ConfigEncoder(json.JSONEncoder):

    def default(self, obj):
        assert isinstance(obj, _Entry)
        return obj.get()


class Config():

    _VERSION = 1
    _MAX_CHARS = 1048576

    def __init__(self):
        self._config_dir = os.path.abspath(
                os.path.expanduser(os.path.join('~', '.kunquat', 'tracker')))
        self._config_path = os.path.join(self._config_dir, 'config.json')
        self._modified = False

        def v_version(x):
            return isinstance(x, int) and 1 <= x < 1000

        def v_dir(x):
            return (x == None) or (isinstance(x, str) and len(x) < 1024)

        def v_style(x):
            if x == None:
                return True
            if not isinstance(x, dict):
                return False

            def v_colour(x):
                return (isinstance(x, str) and
                        len(x) in (4, 7) and
                        x[0] == '#' and
                        all(c in string.hexdigits for c in x[1:]))

            colour_names = [
                'bg_colour',
                'fg_colour',
                'bg_sunken_colour',
                'disabled_fg_colour',
                'important_button_bg_colour',
                'important_button_fg_colour',
                'active_indicator_colour',
                'conns_bg_colour',
                'conns_focus_colour',
                'conns_edge_colour',
                'conns_port_colour',
                'conns_invalid_port_colour',
                'conns_inst_bg_colour',
                'conns_inst_fg_colour',
                'conns_effect_bg_colour',
                'conns_effect_fg_colour',
                'conns_proc_voice_bg_colour',
                'conns_proc_voice_fg_colour',
                'conns_proc_voice_selected_colour',
                'conns_proc_mixed_bg_colour',
                'conns_proc_mixed_fg_colour',
                'conns_master_bg_colour',
                'conns_master_fg_colour',
                'envelope_bg_colour',
                'envelope_axis_label_colour',
                'envelope_axis_line_colour',
                'envelope_curve_colour',
                'envelope_node_colour',
                'envelope_focus_colour',
                'envelope_loop_marker_colour',
                'peak_meter_bg_colour',
                'peak_meter_low_colour',
                'peak_meter_mid_colour',
                'peak_meter_high_colour',
                'peak_meter_clip_colour',
                'position_bg_colour',
                'position_fg_colour',
                'position_stopped_colour',
                'position_play_colour',
                'position_record_colour',
                'position_infinite_colour',
                'position_title_colour',
                'sample_map_bg_colour',
                'sample_map_axis_label_colour',
                'sample_map_axis_line_colour',
                'sample_map_focus_colour',
                'sample_map_point_colour',
                'sample_map_selected_colour',
                'sheet_area_selection_colour',
                'sheet_canvas_bg_colour',
                'sheet_column_bg_colour',
                'sheet_column_border_colour',
                'sheet_cursor_view_line_colour',
                'sheet_cursor_edit_line_colour',
                'sheet_grid_level_1_colour',
                'sheet_grid_level_2_colour',
                'sheet_grid_level_3_colour',
                'sheet_header_bg_colour',
                'sheet_header_fg_colour',
                'sheet_header_solo_colour',
                'sheet_ruler_bg_colour',
                'sheet_ruler_fg_colour',
                'sheet_ruler_playback_marker_colour',
                'sheet_playback_cursor_colour',
                'sheet_trigger_default_colour',
                'sheet_trigger_note_on_colour',
                'sheet_trigger_hit_colour',
                'sheet_trigger_note_off_colour',
                'sheet_trigger_warning_bg_colour',
                'sheet_trigger_warning_fg_colour',
                'system_load_bg_colour',
                'system_load_low_colour',
                'system_load_mid_colour',
                'system_load_high_colour',
                'text_bg_colour',
                'text_fg_colour',
                'text_selected_bg_colour',
                'text_selected_fg_colour',
                'text_disabled_fg_colour',
                'waveform_bg_colour',
                'waveform_focus_colour',
                'waveform_centre_line_colour',
                'waveform_zoomed_out_colour',
                'waveform_single_item_colour',
                'waveform_interpolated_colour',
                'waveform_loop_marker_colour',
            ]

            style_config = {
                'enabled': _Entry(lambda x: isinstance(x, bool), False),
                'def_font_size': _Entry(lambda x: isinstance(x, (int, float)), 0),
                'def_font_family': _Entry(lambda x: isinstance(x, str), ''),
                'border_contrast': _Entry(lambda x: 0 <= x <= 1, 0.3),
                'button_brightness': _Entry(lambda x: -1 <= x <= 1, 0),
                'button_press_brightness': _Entry(lambda x: -1 <= x <= 1, 0),
            }
            for name in colour_names:
                assert name.endswith('_colour')
                style_config[name] = _Entry(v_colour, '#000')

            is_valid = all(k not in style_config or style_config[k].accepts(v)
                    for (k, v) in x.items())
            return is_valid

        self._config = {
            'version'               : _Entry(v_version, self._VERSION),
            'input_control_view'    : _Entry(lambda x: x in ('full', 'compact'), 'full'),
            'chord_mode'            : _Entry(lambda x: isinstance(x, bool), False),
            'dir_effects'           : _Entry(v_dir, None),
            'dir_instruments'       : _Entry(v_dir, None),
            'dir_modules'           : _Entry(v_dir, None),
            'dir_samples'           : _Entry(v_dir, None),
            'follow_playback_cursor': _Entry(lambda x: type(x) == bool, False),
            'style'                 : _Entry(v_style, {}),
        }

    def get_value(self, key):
        return self._config[key].get()

    def set_value(self, key, value):
        if value != self._config[key].get():
            self._modified = True
            self._config[key].set(value)

    def _get_validated_config(self, unsafe_data):
        safe_data = {}

        # Check that we have a JSON dictionary
        try:
            decoded = json.loads(unsafe_data)
        except ValueError as e:
            print('Configuration is not valid JSON data: {}'.format(str(e)),
                    file=sys.stderr)
            return safe_data

        if type(decoded) != dict:
            print('Configuration is not a JSON dictionary', file=sys.stderr)
            return safe_data

        # Get configuration data
        stored_version = decoded.get('version', self._VERSION)
        if not self._config['version'].accepts(stored_version):
            stored_version = self._VERSION

        for k, v in decoded.items():
            if k == 'version':
                continue
            if k in self._config and self._config[k].accepts(v):
                safe_data[k] = self._config[k].get_modified(v)

        return safe_data

    def _load_config_data(self, f):
        chunks = []
        chars_read = 0
        while chars_read <= self._MAX_CHARS:
            chunk = f.read(4096)
            if not chunk:
                break
            chunks.append(chunk)
            chars_read += len(chunk)
        return chunks

    def try_load(self):
        chunks = []

        # Read file
        try:
            with open(self._config_path, encoding='utf-8') as f:
                chunks = self._load_config_data(f)
        except FileNotFoundError:
            old_path = self._config_path + '.old'
            try:
                with open(old_path, encoding='utf-8') as f:
                    chunks = self._load_config_data(f)
            except FileNotFoundError:
                # Nothing to do, using default settings
                return
            except OSError as e:
                # TODO: Should we report this failure?
                return
        except OSError as e:
            print('Could not open tracker configuration at {}: {}'.format(
                self._config_path, e.strerror),
                file=sys.stderr)
            return

        # Safety check
        chars_read = sum(len(c) for c in chunks)
        if chars_read > self._MAX_CHARS:
            print('Tracker configuration at {} is too large, ignoring'.format(
                self._config_path),
                file=sys.stderr)
            return

        # Apply valid settings
        config_data = self._get_validated_config(''.join(chunks))
        self._config.update(config_data)

    def try_save(self):
        if not self._modified:
            return

        # Make sure that our destination directory exists
        if not os.path.exists(self._config_dir):
            try:
                os.makedirs(self._config_dir)
            except OSError as e:
                print('Could not create directory for tracker configuration'
                        ' at {}: {}'.format(self._config_dir, e.strerror),
                        file=sys.stderr)
                return

        if not os.path.isdir(self._config_dir):
            print('Could not create directory for tracker configuration at {}: path'
                    ' already exists but is not a directory'.format(self._config_dir),
                    file=sys.stderr)
            return

        new_path = self._config_path + '.new'
        old_path = self._config_path + '.old'

        # Write new configuration file
        out_data = json.dumps(self._config, indent=4, cls=_ConfigEncoder, sort_keys=True)
        assert len(out_data) <= self._MAX_CHARS
        try:
            with open(new_path, 'w', encoding='utf-8') as f:
                f.write(out_data)
        except OSError as e:
            print('Could not write new configuration file: {}'.format(e.strerror),
                    file=sys.stderr)
            return

        # Replace old configuration with the new one
        try:
            if os.path.exists(self._config_path):
                os.replace(self._config_path, old_path)
            os.replace(new_path, self._config_path)
            if os.path.exists(old_path):
                os.remove(old_path)
        except OSError as e:
            print('An error occurred while replacing old configuration file: {}'.format(
                e.strerror,
                file=sys.stderr))
            return


_config = Config()


def get_config():
    return _config


def try_load():
    _config.try_load()


def try_save():
    _config.try_save()


