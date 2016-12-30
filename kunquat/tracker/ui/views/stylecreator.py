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

import os.path
import re

from PySide.QtCore import *
from PySide.QtGui import *


class StyleCreator():

    def __init__(self):
        self._ui_model = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def unregister_updaters(self):
        pass

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

    def _adjust_brightness(self, colour, add):
        return tuple(c + add for c in colour)

    def update_style_sheet(self):
        style_manager = self._ui_model.get_style_manager()
        app = QApplication.instance()

        if not style_manager.is_custom_style_enabled():
            app.setStyleSheet(style_manager.get_init_style_sheet())
            return

        icon_bank = self._ui_model.get_icon_bank()

        # Get colours from the configuration
        contrast = 0.3
        grad = -0.07
        button_down = -0.15

        icons_dir = style_manager.get_icons_dir()
        icons_path = '/'.join(os.path.split(icons_dir))

        bg_colour_str = style_manager.get_style_param('bg_colour', '#db9')
        fg_colour_str = style_manager.get_style_param('fg_colour', '#000')

        disabled_fg_colour = self._get_colour_from_str(
                style_manager.get_style_param('disabled_fg_colour', '#543'))

        button_bg_colour_str = style_manager.get_style_param('button_bg_colour', '#b97')
        button_fg_colour_str = style_manager.get_style_param('button_fg_colour', '#000')

        bg_colour = self._get_colour_from_str(bg_colour_str)
        fg_colour = self._get_colour_from_str(fg_colour_str)

        button_bg_colour = self._get_colour_from_str(button_bg_colour_str)
        button_fg_colour = self._get_colour_from_str(button_fg_colour_str)
        button_down_bg_colour = self._adjust_brightness(button_bg_colour, button_down)
        button_down_fg_colour = self._adjust_brightness(button_fg_colour, button_down)

        text_bg_colour = self._get_colour_from_str(
                style_manager.get_style_param('text_bg_colour', '#000'))
        text_fg_colour = self._get_colour_from_str(
                style_manager.get_style_param('text_fg_colour', '#da5'))

        def make_light(colour):
            return self._adjust_brightness(colour, contrast)

        def make_grad(colour):
            return self._adjust_brightness(colour, grad)

        def make_dark(colour):
            return self._adjust_brightness(colour, -contrast)

        # Get derived colours
        colours = {
            'icons_path'                 : icons_path,
            'bg_colour'                  : bg_colour,
            'bg_colour_light'            : make_light(bg_colour),
            'bg_colour_dark'             : make_dark(bg_colour),
            'fg_colour'                  : fg_colour,
            'disabled_fg_colour'         : disabled_fg_colour,
            'button_bg_colour'           : button_bg_colour,
            'button_bg_colour_light'     : make_light(button_bg_colour),
            'button_bg_colour_grad'      : make_grad(button_bg_colour),
            'button_bg_colour_dark'      : make_dark(button_bg_colour),
            'button_down_bg_colour'      : button_down_bg_colour,
            'button_down_bg_colour_light': make_light(button_down_bg_colour),
            'button_down_bg_colour_grad' : make_grad(button_down_bg_colour),
            'button_down_bg_colour_dark' : make_dark(button_down_bg_colour),
            'button_fg_colour'           : button_fg_colour,
            'button_down_fg_colour'      : button_down_fg_colour,
            'scrollbar_bg_colour'        : self._adjust_brightness(bg_colour, -0.2),
            'text_bg_colour'             : text_bg_colour,
            'text_fg_colour'             : text_fg_colour,
        }

        template = style_manager.get_style_sheet_template()

        replacements = {
                '<' + k + '>': (self._get_str_from_colour(v) if type(v) == tuple else v)
                for (k, v) in colours.items() }
        regexp = re.compile('|'.join(re.escape(k) for k in replacements.keys()))
        style_sheet = regexp.sub(lambda match: replacements[match.group(0)], template)

        app.setStyleSheet(style_sheet)


