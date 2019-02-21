# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2019
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
import tempfile
import unicodedata


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
    _THEME_VERSION = 1
    _MAX_CHARS = 1048576

    def __init__(self):
        self._config_dir = os.path.abspath(
                os.path.expanduser(os.path.join('~', '.kunquat', 'tracker')))
        self._config_path = os.path.join(self._config_dir, 'config.json')
        self._themes_dir = os.path.join(self._config_dir, 'themes')
        self._is_modified = False

        def v_dir(x):
            return (x == None) or (isinstance(x, str) and len(x) < 1024)

        def v_theme_id(x):
            if x == None:
                return True
            if (not isinstance(x, (tuple, list))) or (len(x) != 2):
                return False
            if not all(isinstance(a, str) for a in x):
                return False
            if x[0] not in ('default', 'share', 'custom'):
                return False

            allowed_name_chs = string.ascii_lowercase + string.digits + '_'
            if not all(ch in allowed_name_chs for ch in x[1]):
                return False

            return True

        self._config = {
            'version'               : _Entry(self._is_valid_version, self._VERSION),
            'input_control_view'    : _Entry(lambda x: x in ('full', 'compact'), 'full'),
            'chord_mode'            : _Entry(lambda x: isinstance(x, bool), False),
            'dir_effects'           : _Entry(v_dir, None),
            'dir_instruments'       : _Entry(v_dir, None),
            'dir_modules'           : _Entry(v_dir, None),
            'dir_samples'           : _Entry(v_dir, None),
            'follow_playback_cursor': _Entry(lambda x: type(x) == bool, False),
            'selected_theme_id'     : _Entry(v_theme_id, ('default', 'default')),

            'style': _Entry(lambda x: self._is_valid_style(x, is_theme=False), {}),
        }

    def _is_valid_version(self, x):
        return isinstance(x, int) and 1 <= x < 1000

    def _is_valid_style(self, x, is_theme=True):
        if x == None:
            return True
        if not isinstance(x, dict):
            return False

        style_config = self._get_style_config(is_theme)

        is_valid = all(k in style_config and style_config[k].accepts(v)
                for (k, v) in x.items())
        return is_valid

    def _get_style_config(self, is_theme=True):
        def v_colour(x):
            return (isinstance(x, str) and
                    len(x) in (4, 7) and
                    x[0] == '#' and
                    all(c in string.hexdigits for c in x[1:]))

        def v_name(x):
            return (isinstance(x, str) and (len(x) <= 256))

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
            'def_font_size': _Entry(lambda x: 0 <= x <= 50, 0),
            'def_font_family': _Entry(lambda x: isinstance(x, str), ''),
            'border_contrast': _Entry(lambda x: 0 <= x <= 1, 0.3),
            'button_brightness': _Entry(lambda x: -1 <= x <= 1, 0),
            'button_press_brightness': _Entry(lambda x: -1 <= x <= 1, 0),
        }
        for name in colour_names:
            assert name.endswith('_colour')
            style_config[name] = _Entry(v_colour, '#000')

        if is_theme:
            style_config['version'] = _Entry(self._is_valid_version, self._THEME_VERSION)
            style_config['name'] = _Entry(v_name, '')

        return style_config

    def get_value(self, key):
        return self._config[key].get()

    def set_value(self, key, value):
        if value != self._config[key].get():
            self._is_modified = True
            self._config[key].set(value)

    def get_theme_names(self):
        try:
            fnames = (n for n in os.listdir(self._themes_dir) if n.endswith('.json'))
            theme_names = [name.rpartition('.')[0] for name in fnames]
        except OSError:
            theme_names = []

        return theme_names

    def make_theme_name(self, disp_name, unique):
        # Strip out all weirdness in the name
        norm_name = unicodedata.normalize('NFKD', disp_name)
        no_combs = ''.join(c.lower() for c in norm_name if not unicodedata.combining(c))
        allowed_chs = string.ascii_lowercase + string.digits + '_'
        allowed_ch_name = ''.join((c if c in allowed_chs else '_') for c in no_combs)
        parts = allowed_ch_name.split('_')
        well_formed = '_'.join(p for p in parts if p)
        if not well_formed:
            well_formed = 'theme'

        if unique:
            # Make sure the name is unique
            used_names = self.get_theme_names()
            final_name = well_formed
            if well_formed in used_names:
                for i in range(1000):
                    final_name = '{}{}'.format(well_formed, i)
                    if final_name not in used_names:
                        break
                else:
                    try:
                        f, path = tempfile.mkstemp('.json', 't', self._themes_dir)
                        f.close()
                        try:
                            st = os.stat(path)
                            if st.st_size == 0:
                                os.remove(path)
                        except OSError:
                            pass

                        _, filename = os.path.split(path)
                        final_name = filename.rpartition('.')[0]
                    except OSError:
                        return None
        else:
            final_name = well_formed

        return final_name

    def _get_theme_path(self, theme_name):
        return os.path.join(self._themes_dir, theme_name + '.json')

    def rename_theme(self, old_name, new_name):
        old_path = self._get_theme_path(old_name)
        new_path = self._get_theme_path(new_name)

        if not os.path.exists(old_path):
            print('Theme path to be renamed {} does not exist'.format(old_path))
            return False

        if os.path.exists(new_path):
            print('Theme path for new name {} already exists'.format(new_path))
            return False

        try:
            os.rename(old_path, new_path)
        except OSError as e:
            print('Could not rename {} to {}: {}'.format(old_name, new_name, e.strerror))
            return False

        return True

    def _try_load_theme(self, theme_name):
        theme_path = self._get_theme_path(theme_name)

        decoded, success = self._load_json_data(theme_path, is_theme=True)
        if not success:
            return None

        theme_config = self._get_style_config()

        stored_version = decoded.get('version', self._THEME_VERSION)
        if not theme_config['version'].accepts(stored_version):
            stored_version = self._THEME_VERSION

        safe_data = {}
        for k, v in decoded.items():
            if k == 'version':
                continue
            if k in theme_config and theme_config[k].accepts(v):
                safe_data[k] = theme_config[k].get_modified(v)

        return safe_data

    def _try_save_theme(self, theme_name, theme):
        theme_path = self._get_theme_path(theme_name)

        # Make sure that our destination directory exists
        if not self._check_make_dir(self._themes_dir, is_theme=True):
            return False

        return self._save_file(theme_path, theme, is_theme=True)

    def _try_save_modified_theme(self):
        if self._is_theme_modified:
            assert self._theme_name
            self._try_save_theme(self._theme_name, self._theme_config)
            self._theme_name = None
            self._theme_config = self._get_style_config()
            self._is_theme_modified = False

    def get_theme(self, theme_name):
        theme = self._try_load_theme(theme_name)
        if not theme:
            return {}

        converted = { k: v.get() for (k, v) in theme.items() }
        return converted

    def set_theme(self, theme_name, theme):
        assert self._is_valid_style(theme)
        return self._try_save_theme(theme_name, theme)

    def remove_theme(self, theme_name):
        theme_path = self._get_theme_path(theme_name)
        try:
            os.remove(theme_path)
        except OSError as e:
            print('Could not remove theme at {}: {}'.format(theme_path, e.strerror))

    def _get_validated_config(self, unsafe_data):
        safe_data = {}

        # Get configuration data
        stored_version = unsafe_data.get('version', self._VERSION)
        if not self._config['version'].accepts(stored_version):
            stored_version = self._VERSION

        for k, v in unsafe_data.items():
            if k == 'version':
                continue
            if k in self._config and self._config[k].accepts(v):
                safe_data[k] = self._config[k].get_modified(v)

        return safe_data

    def _load_chunks(self, f):
        chunks = []
        chars_read = 0
        while chars_read <= self._MAX_CHARS:
            chunk = f.read(4096)
            if not chunk:
                break
            chunks.append(chunk)
            chars_read += len(chunk)
        return chunks

    def _load_json_data(self, path, is_theme):
        chunks = []

        kind = 'theme' if is_theme else 'tracker configuration'

        # Read file
        try:
            with open(path, encoding='utf-8') as f:
                chunks = self._load_chunks(f)
        except FileNotFoundError:
            old_path = path + '_.old'
            try:
                with open(old_path, encoding='utf-8') as f:
                    chunks = self._load_chunks(f)
            except FileNotFoundError:
                # Nothing to do, using default values
                return {}, True
            except OSError as e:
                # TODO: Should we report this failure?
                return None, False
        except OSError as e:
            print('Could not open {} at {}: {}'.format(kind, path, e.strerror),
                    file=sys.stderr)
            return None, False

        # Safety check
        chars_read = sum(len(c) for c in chunks)
        if chars_read > self._MAX_CHARS:
            print('{} at {} is too large, ignoring'.format(kind.capitalize(), path),
                    file=sys.stderr)
            return None, False

        unsafe_data = ''.join(chunks)

        # Check that we have a JSON dictionary
        try:
            decoded = json.loads(unsafe_data)
        except ValueError as e:
            print('{} is not valid JSON data: {}'.format(kind.capitalize(), str(e)),
                    file=sys.stderr)
            return None, False

        if type(decoded) != dict:
            print('{} is not a JSON dictionary'.format(kind.capitalize()),
                    file=sys.stderr)
            return None, False

        return decoded, True

    def try_load(self):
        jdata, success = self._load_json_data(self._config_path, is_theme=False)
        if not success:
            return

        # Apply valid settings
        config_data = self._get_validated_config(jdata)
        self._config.update(config_data)

    def _check_make_dir(self, path, is_theme):
        kind = 'themes' if is_theme else 'tracker configuration'

        if not os.path.exists(path):
            try:
                os.makedirs(path)
            except OSError as e:
                print('Could not create directory for {} at {}: {}'.format(
                    kind, path, e.strerror),
                    file=sys.stderr)
                return False

        if not os.path.isdir(path):
            print('Could not create directory for {} at {}: path'
                    ' already exists but is not a directory'.format(kind, path),
                    file=sys.stderr)
            return False

        return True

    def _save_file(self, path, config, is_theme):
        kind = 'theme' if is_theme else 'configuration'

        new_path = path + '_.new'
        old_path = path + '_.old'

        # Write new data
        out_data = json.dumps(config, indent=4, cls=_ConfigEncoder, sort_keys=True)
        assert len(out_data) <= self._MAX_CHARS
        try:
            with open(new_path, 'w', encoding='utf-8') as f:
                f.write(out_data)
        except OSError as e:
            print('Could not write new {} file: {}'.format(kind, e.strerror),
                    file=sys.stderr)
            return False

        # Replace old file with the new one
        try:
            if os.path.exists(path):
                os.replace(path, old_path)
            os.replace(new_path, path)
            if os.path.exists(old_path):
                os.remove(old_path)
        except OSError as e:
            print('An error occurred while replacing old {} file: {}'.format(
                kind, e.strerror,
                file=sys.stderr))
            return False

        return True

    def try_save(self):
        if not self._is_modified:
            return

        # Make sure that our destination directory exists
        if not self._check_make_dir(self._config_dir, is_theme=False):
            return

        self._save_file(self._config_path, self._config, is_theme=False)


_config = Config()


def get_config():
    return _config


def try_load():
    _config.try_load()


def try_save():
    _config.try_save()


