# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import kunquat.tracker.config as config


class StyleManager():

    _STYLE_DEFAULTS = {
        'bg_colour'                      : '#db9',
        'fg_colour'                      : '#000',
        'bg_colour_sunken'               : '#a86',
        'disabled_fg_colour'             : '#543',
        'button_bg_colour'               : '#b97',
        'button_fg_colour'               : '#000',
        'sheet_area_selection_colour'    : '#8ac',
        'sheet_canvas_bg_colour'         : '#111',
        'sheet_column_bg_colour'         : '#000',
        'sheet_column_border_colour'     : '#555',
        'sheet_cursor_view_line_colour'  : '#def',
        'sheet_cursor_edit_line_colour'  : '#f84',
        'sheet_grid_level_1_colour'      : '#aaa',
        'sheet_grid_level_2_colour'      : '#666',
        'sheet_grid_level_3_colour'      : '#444',
        'sheet_header_bg_colour'         : '#242',
        'sheet_header_fg_colour'         : '#cea',
        'sheet_header_border_colour'     : '#575',
        'sheet_ruler_bg_colour'          : '#125',
        'sheet_ruler_fg_colour'          : '#acf',
        'sheet_trigger_default_colour'   : '#cde',
        'sheet_trigger_note_on_colour'   : '#fdb',
        'sheet_trigger_hit_colour'       : '#be8',
        'sheet_trigger_note_off_colour'  : '#c96',
        'sheet_trigger_warning_bg_colour': '#e31',
        'sheet_trigger_warning_fg_colour': '#ffc',
        'text_bg_colour'                 : '#000',
        'text_fg_colour'                 : '#da5',
        'position_bg_colour'             : '#000',
        'position_fg_colour'             : '#6d6',
        'position_stopped_colour'        : '#555',
        'position_play_colour'           : '#6d6',
        'position_record_colour'         : '#d43',
        'position_infinite_colour'       : '#fd5',
        'position_title_colour'          : '#777',
        'typewriter_active_note_colour'  : '#f00',
    }

    def __init__(self):
        self._controller = None
        self._ui_model = None
        self._share = None
        self._init_ss = None

    def set_controller(self, controller):
        self._controller = controller
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

    def set_custom_style_enabled(self, enabled):
        config_style = self._get_config_style()
        config_style['enabled'] = enabled
        self._set_config_style(config_style)

    def is_custom_style_enabled(self):
        config_style = self._get_config_style()
        return config_style.get('enabled', False)

    def get_style_param(self, key):
        config_style = self._get_config_style()
        return config_style.get(key, self._STYLE_DEFAULTS[key])

    def set_style_param(self, key, value):
        config_style = self._get_config_style()
        config_style[key] = value
        self._set_config_style(config_style)

    def _set_config_style(self, style):
        cfg = config.get_config()
        cfg.set_value('style', style)

    def _get_config_style(self):
        cfg = config.get_config()
        return cfg.get_value('style') or {}


