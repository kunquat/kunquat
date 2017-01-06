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
        'conns_bg_colour'                : '#111',
        'conns_edge_colour'              : '#ccc',
        'conns_focus_colour'             : '#f72',
        'conns_port_colour'              : '#eca',
        'conns_invalid_port_colour'      : '#f33',
        'conns_inst_bg_colour'           : '#335',
        'conns_inst_fg_colour'           : '#def',
        'conns_inst_button_bg_colour'    : '#113',
        'conns_effect_bg_colour'         : '#543',
        'conns_effect_fg_colour'         : '#fed',
        'conns_effect_button_bg_colour'  : '#321',
        'conns_proc_voice_bg_colour'     : '#255',
        'conns_proc_voice_fg_colour'     : '#cff',
        'conns_proc_voice_button_bg_colour': '#133',
        'conns_proc_voice_hilight_selected': '#9b9',
        'conns_proc_mixed_bg_colour'     : '#525',
        'conns_proc_mixed_fg_colour'     : '#fcf',
        'conns_proc_mixed_button_bg_colour': '#313',
        'conns_master_bg_colour'         : '#353',
        'conns_master_fg_colour'         : '#dfd',
        'peak_meter_bg_colour'           : '#000',
        'peak_meter_low_colour'          : '#191',
        'peak_meter_mid_colour'          : '#dc3',
        'peak_meter_high_colour'         : '#e21',
        'peak_meter_clip_colour'         : '#f32',
        'position_bg_colour'             : '#000',
        'position_fg_colour'             : '#6d6',
        'position_stopped_colour'        : '#555',
        'position_play_colour'           : '#6d6',
        'position_record_colour'         : '#d43',
        'position_infinite_colour'       : '#fd5',
        'position_title_colour'          : '#777',
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


