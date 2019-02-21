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
import itertools
import re

import kunquat.tracker.config as config


class StyleManager():

    _STYLE_DEFAULTS = {
        'border_thick_radius'               : 0.4,
        'border_thick_width'                : 0.2,
        'border_thin_radius'                : 0.2,
        'border_thin_width'                 : 0.1,
        'combobox_arrow_size'               : 0.8,
        'combobox_button_size'              : 0.6,
        'dialog_button_width'               : 9.0,
        'dialog_icon_size'                  : 7.0,
        'file_dialog_icon_size'             : 1.5,
        'large_padding'                     : 0.8,
        'list_button_size'                  : 2.0,
        'list_button_padding'               : 0.2,
        'medium_padding'                    : 0.4,
        'menu_arrow_size'                   : 1.0,
        'radio_border_radius'               : 0.499,
        'radio_check_size'                  : 1.0,
        'radio_check_spacing'               : 0.5,
        'scrollbar_margin'                  : 1.1,
        'scrollbar_size'                    : 1.2,
        'sheet_grid_line_width'             : 0.1,
        'slider_handle_size'                : 3.0,
        'slider_thickness'                  : 1.0,
        'small_padding'                     : 0.2,
        'splitter_width'                    : 0.4,
        'tab_bar_margin'                    : 0.4,
        'tiny_arrow_button_size'            : 0.9,
        'tiny_padding'                      : 0.1,
        'tool_button_size'                  : 3.4,
        'tool_button_padding'               : 0.62,
        'typewriter_button_size'            : 5.6,
        'typewriter_padding'                : 2.9,
    }

    _STYLE_COLOUR_DEFAULTS = {
        'bg_colour'                         : '#4c474e',
        'fg_colour'                         : '#dbdbdb',
        'bg_sunken_colour'                  : '#2b2238',
        'disabled_fg_colour'                : '#969696',
        'important_button_bg_colour'        : '#d5412c',
        'important_button_fg_colour'        : '#fff',
        'active_indicator_colour'           : '#ff2020',
        'conns_bg_colour'                   : '#111',
        'conns_edge_colour'                 : '#ccc',
        'conns_focus_colour'                : '#f72',
        'conns_port_colour'                 : '#eca',
        'conns_invalid_port_colour'         : '#f33',
        'conns_inst_bg_colour'              : '#335',
        'conns_inst_fg_colour'              : '#def',
        'conns_effect_bg_colour'            : '#543',
        'conns_effect_fg_colour'            : '#fed',
        'conns_proc_voice_bg_colour'        : '#255',
        'conns_proc_voice_fg_colour'        : '#cff',
        'conns_proc_voice_selected_colour'  : '#9b9',
        'conns_proc_mixed_bg_colour'        : '#525',
        'conns_proc_mixed_fg_colour'        : '#fcf',
        'conns_master_bg_colour'            : '#353',
        'conns_master_fg_colour'            : '#dfd',
        'envelope_bg_colour'                : '#000',
        'envelope_axis_label_colour'        : '#ccc',
        'envelope_axis_line_colour'         : '#ccc',
        'envelope_curve_colour'             : '#68a',
        'envelope_node_colour'              : '#eca',
        'envelope_focus_colour'             : '#f72',
        'envelope_loop_marker_colour'       : '#7799bb',
        'peak_meter_bg_colour'              : '#000',
        'peak_meter_low_colour'             : '#191',
        'peak_meter_mid_colour'             : '#ddcc33',
        'peak_meter_high_colour'            : '#e21',
        'peak_meter_clip_colour'            : '#f32',
        'position_bg_colour'                : '#000',
        'position_fg_colour'                : '#6d6',
        'position_stopped_colour'           : '#555',
        'position_play_colour'              : '#6d6',
        'position_record_colour'            : '#d43',
        'position_infinite_colour'          : '#fd5',
        'position_title_colour'             : '#777',
        'sample_map_bg_colour'              : '#000',
        'sample_map_axis_label_colour'      : '#ccc',
        'sample_map_axis_line_colour'       : '#ccc',
        'sample_map_focus_colour'           : '#f72',
        'sample_map_point_colour'           : '#eca',
        'sample_map_selected_colour'        : '#ffd',
        'sheet_area_selection_colour'       : '#8ac',
        'sheet_canvas_bg_colour'            : '#111',
        'sheet_column_bg_colour'            : '#000',
        'sheet_column_border_colour'        : '#222',
        'sheet_cursor_view_line_colour'     : '#def',
        'sheet_cursor_edit_line_colour'     : '#f84',
        'sheet_grid_level_1_colour'         : '#aaa',
        'sheet_grid_level_2_colour'         : '#666',
        'sheet_grid_level_3_colour'         : '#444',
        'sheet_header_bg_colour'            : '#242',
        'sheet_header_fg_colour'            : '#cea',
        'sheet_header_solo_colour'          : '#7e6',
        'sheet_ruler_bg_colour'             : '#125',
        'sheet_ruler_fg_colour'             : '#acf',
        'sheet_ruler_playback_marker_colour': '#d68',
        'sheet_playback_cursor_colour'      : '#6e6',
        'sheet_trigger_default_colour'      : '#cde',
        'sheet_trigger_note_on_colour'      : '#fdb',
        'sheet_trigger_hit_colour'          : '#be8',
        'sheet_trigger_note_off_colour'     : '#c96',
        'sheet_trigger_warning_bg_colour'   : '#e31',
        'sheet_trigger_warning_fg_colour'   : '#ffc',
        'system_load_bg_colour'             : '#000',
        'system_load_low_colour'            : '#191',
        'system_load_mid_colour'            : '#dc3',
        'system_load_high_colour'           : '#e21',
        'text_bg_colour'                    : '#211d2c',
        'text_fg_colour'                    : '#d9c7a9',
        'text_selected_bg_colour'           : '#36a',
        'text_selected_fg_colour'           : '#ffc',
        'text_disabled_fg_colour'           : '#8b7865',
        'waveform_bg_colour'                : '#000',
        'waveform_focus_colour'             : '#fa5',
        'waveform_centre_line_colour'       : '#666',
        'waveform_zoomed_out_colour'        : '#67d091',
        'waveform_single_item_colour'       : '#67d091',
        'waveform_interpolated_colour'      : '#396',
        'waveform_loop_marker_colour'       : '#dfd58e',
    }

    _STYLE_CONFIG_DEFAULTS = {
        'def_font_size'                     : 0,
        'def_font_family'                   : '',
        'border_contrast'                   : 0.25,
        'button_brightness'                 : 0.10,
        'button_press_brightness'           : -0.2,
    }

    _STYLE_CONFIG_DEFAULTS.update(_STYLE_COLOUR_DEFAULTS)
    _STYLE_DEFAULTS.update(_STYLE_CONFIG_DEFAULTS)

    _THEME_DEFAULT = ('default', 'default')

    def __init__(self):
        self._controller = None
        self._ui_model = None
        self._session = None
        self._share = None
        self._init_ss = None

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._share = controller.get_share()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def set_init_style_sheet(self, init_ss):
        self._init_ss = init_ss

    def get_init_style_sheet(self):
        return self._init_ss

    def get_style_sheet_template(self):
        return self._share.get_style_sheet('default')

    def get_icons_dir(self):
        return self._share.get_icons_dir()

    def get_style_param(self, key):
        config_style = self._get_config_style()
        return config_style.get(key, self._STYLE_DEFAULTS[key])

    def try_flush_cached_style(self):
        cached_theme_id = self._session.cached_theme_id
        if (not cached_theme_id) or (not self._session.is_cached_theme_modified):
            return

        assert cached_theme_id[0] == 'custom'

        cfg = config.get_config()
        if cfg.set_theme(cached_theme_id[1], self._session.cached_theme):
            self._session.is_cached_theme_modified = False
        else:
            print('Could not save cached theme')

    def set_style_param(self, key, value):
        assert self.get_selected_theme_id()[0] == 'custom'

        if self._session.cached_theme_id != self.get_selected_theme_id():
            self.try_flush_cached_style()

        config_style = self._get_config_style()

        config_style[key] = value
        if key.endswith('_colour'):
            # Make sure that all colours are stored if one is changed
            for k in self._STYLE_COLOUR_DEFAULTS.keys():
                if (k != key) and (k not in config_style):
                    config_style[k] = self._STYLE_COLOUR_DEFAULTS[k]

        self._session.cached_theme_id = self.get_selected_theme_id()
        self._session.cached_theme = config_style
        self._session.is_cached_theme_modified = True

    def set_reference_font_height(self, height):
        self._session.set_reference_font_height(height)

    def get_scaled_size(self, size_norm, min_size=1):
        ref_height = self._session.get_reference_font_height()
        return max(min_size, int(round(size_norm * ref_height)))

    def get_scaled_size_param(self, size_param, min_size=1):
        size_norm = self.get_style_param(size_param)
        return self.get_scaled_size(size_norm, min_size)

    def get_colour_param_intensity(self, param):
        colour = self._get_colour_from_str(self.get_style_param(param))
        return self.get_colour_intensity(colour)

    def get_colour_intensity(self, colour):
        return (colour[0] * 0.212) + (colour[1] * 0.715) + (colour[2] * 0.072)

    def get_adjusted_colour(self, param, brightness):
        orig_colour = self._get_colour_from_str(self.get_style_param(param))
        adjusted_colour = (c + brightness for c in orig_colour)
        return self._get_str_from_colour(adjusted_colour)

    def get_link_colour(self, colour_param='fg_colour'):
        shift = (-0.3, 0.1, 0.6)
        fg_colour = self._get_colour_from_str(self.get_style_param(colour_param))
        return self._get_str_from_colour(c + s for (c, s) in zip(fg_colour, shift))

    def get_help_style(self, font_size):
        share = self._controller.get_share()
        stored_style = share.get_help_style()

        substs = {
            '{': '{{',
            '}': '}}',
            '<': '{',
            '>': '}',
        }

        patterns = ['/\*.*?\*/'] + [re.escape(k) for k in substs.keys()]
        preformat = re.compile('|'.join(patterns), re.DOTALL)
        pref_style = preformat.sub(lambda mo: substs.get(mo.group(0), ''), stored_style)

        kwargs = {
            'pt': _SizeHelper(font_size, 'pt'),
            'px': _SizeHelper(self.get_scaled_size(1), 'px'),
            'col': _ColourHelper(self),
        }

        final_style = pref_style.format(**kwargs)
        return final_style

    def get_stock_theme_ids(self):
        names = self._share.get_theme_names()
        return [StyleManager._THEME_DEFAULT] + [('share', n) for n in names]

    def get_custom_theme_ids(self):
        cfg = config.get_config()
        return [('custom', name) for name in cfg.get_theme_names()]

    def set_selected_theme_id(self, theme_id):
        cfg = config.get_config()
        cfg.set_value('selected_theme_id', theme_id)

    def get_selected_theme_id(self):
        cfg = config.get_config()
        stored_theme_id = cfg.get_value('selected_theme_id')
        if not stored_theme_id:
            return ('default', 'default')
        return tuple(stored_theme_id)

    def create_new_theme(self):
        cur_theme_data = self.get_theme_data(self.get_selected_theme_id(), cache=False)
        if 'name' in cur_theme_data:
            new_name = 'Copy of {}'.format(cur_theme_data['name'])
        else:
            new_name = 'New theme'

        new_theme_data = dict(StyleManager._STYLE_CONFIG_DEFAULTS)
        new_theme_data.update(cur_theme_data)
        new_theme_data['name'] = new_name

        cfg = config.get_config()
        new_theme_id = ('custom', cfg.make_theme_name(new_name))
        if not new_theme_id[1]:
            print('could not create a new theme ID')
            return

        cfg.set_theme(new_theme_id[1], new_theme_data)

        return new_theme_id

    def get_theme_data(self, theme_id, cache=True):
        theme_type, theme_name = theme_id

        if theme_type == 'share':
            theme = self._share.get_theme(theme_name)
            return theme[key]

        elif theme_type == 'custom':
            if self._session.cached_theme_id == theme_id:
                return self._session.cached_theme
            theme = config.get_config().get_theme(theme_name)
            if cache:
                self.try_flush_cached_style()
                self._session.cached_theme_id = theme_id
                self._session.cached_theme = theme
            return theme

        elif theme_type == 'default':
            theme = dict(StyleManager._STYLE_CONFIG_DEFAULTS)
            theme['name'] = 'Default'
            return theme

        assert False

    def get_theme_name(self, theme_id):
        return self.get_theme_data(theme_id, cache=False).get('name') or ''

    def remove_theme(self, theme_id):
        theme_type, theme_name = theme_id
        assert theme_type == 'custom'
        config.get_config().remove_theme(theme_name)

    def _is_theme_colours_only(self, theme):
        allowed_keys = (
                'name',
                'border_contrast',
                'button_brightness',
                'button_press_brightness')
        return all((k.endswith('_colour') or (k in allowed_keys)) for k in theme.keys())

    def is_theme_stock(self, theme_id):
        theme_type, _ = theme_id
        return (theme_type != 'custom')

    def _get_colour_from_str(self, s):
        if len(s) == 4:
            cs = [s[1], s[2], s[3]]
            colour = tuple(int(c, 16) / 15 for c in cs)
        elif len(s) == 7:
            cs = [s[1:3], s[3:5], s[5:7]]
            colour = tuple(int(c, 16) / 255 for c in cs)
        else:
            assert False
        return colour

    def _get_str_from_colour(self, colour):
        clamped = [min(max(0, c), 1) for c in colour]
        cs = ['{:02x}'.format(int(c * 255)) for c in clamped]
        s = '#' + ''.join(cs)
        assert len(s) == 7
        return s

    def _get_config_style(self):
        return self.get_theme_data(self.get_selected_theme_id()) or {}


class _SizeHelper():

    def __init__(self, default_size, suffix):
        self._size = default_size
        self._suffix = suffix

    def __getitem__(self, index):
        rel_size = float(index)
        abs_size = int(round(self._size * rel_size))
        return '{}{}'.format(abs_size, self._suffix)


class _ColourHelper():

    def __init__(self, style_mgr):
        self._style_mgr = style_mgr

    def __getitem__(self, name):
        if name == 'link_fg':
            return self._style_mgr.get_link_colour('text_fg_colour')
        elif name == 'table_even_bg':
            bg_intensity = self._style_mgr.get_colour_param_intensity('text_bg_colour')
            brightness = 0.12 if bg_intensity < 0.5 else -0.12
            adjusted = self._style_mgr.get_adjusted_colour('text_bg_colour', brightness)
            return adjusted

        colour_param = '{}_colour'.format(name)
        return self._style_mgr.get_style_param(colour_param)


