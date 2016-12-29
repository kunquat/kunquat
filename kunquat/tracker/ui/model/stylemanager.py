# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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

    def __init__(self):
        self._ui_model = None
        self._default_ss = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def set_default_style_sheet(self, default_ss):
        self._default_ss = default_ss

    def get_default_style_sheet(self):
        return self._default_ss

    def set_custom_style_enabled(self, enabled):
        config_style = self._get_config_style()
        config_style['enabled'] = enabled
        self._set_config_style(config_style)

    def is_custom_style_enabled(self):
        config_style = self._get_config_style()
        return config_style.get('enabled', False)

    def get_style_param(self, key, default):
        config_style = self._get_config_style()
        return config_style.get(key, default)

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


