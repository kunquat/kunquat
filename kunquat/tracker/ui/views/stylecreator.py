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

import os.path
import re

from kunquat.tracker.ui.qt import *

from .utils import get_default_font_info


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

    def get_updated_style_sheet(self):
        style_mgr = self._ui_model.get_style_manager()

        if not style_mgr.is_custom_style_enabled():
            return style_mgr.get_init_style_sheet()

        icon_bank = self._ui_model.get_icon_bank()

        # Get font settings
        def_font_family, def_font_size = get_default_font_info(style_mgr)

        # Get colours from the configuration
        contrast = style_mgr.get_style_param('border_contrast')
        grad = -0.07
        button_brightness = style_mgr.get_style_param('button_brightness')
        button_down = style_mgr.get_style_param('button_press_brightness')
        button_down2 = 1.5 * button_down
        tab_shade_top = -0.1
        tab_shade_bottom = -0.25

        icons_dir = style_mgr.get_icons_dir()
        icons_path = '/'.join(os.path.split(icons_dir))

        bg_colour_str = style_mgr.get_style_param('bg_colour')
        fg_colour_str = style_mgr.get_style_param('fg_colour')
        bg_sunken_colour_str = style_mgr.get_style_param('bg_sunken_colour')

        disabled_fg_colour = self._get_colour_from_str(
                style_mgr.get_style_param('disabled_fg_colour'))

        bg_colour = self._get_colour_from_str(bg_colour_str)
        fg_colour = self._get_colour_from_str(fg_colour_str)
        bg_sunken_colour = self._get_colour_from_str(bg_sunken_colour_str)

        button_bg_colour = self._adjust_brightness(bg_colour, button_brightness)
        button_fg_colour = fg_colour
        button_down_bg_colour = self._adjust_brightness(button_bg_colour, button_down)
        button_down_fg_colour = self._adjust_brightness(button_fg_colour, button_down)
        button_down2_bg_colour = self._adjust_brightness(button_bg_colour, button_down2)
        button_down2_fg_colour = self._adjust_brightness(button_fg_colour, button_down2)

        ib_bg_colour = self._get_colour_from_str(
                style_mgr.get_style_param('important_button_bg_colour'))
        ib_fg_colour = self._get_colour_from_str(
                style_mgr.get_style_param('important_button_fg_colour'))
        ib_down_bg_colour = self._adjust_brightness(ib_bg_colour, button_down)
        ib_down_fg_colour = self._adjust_brightness(ib_fg_colour, button_down)
        ib_down2_bg_colour = self._adjust_brightness(ib_bg_colour, button_down2)
        ib_down2_fg_colour = self._adjust_brightness(ib_fg_colour, button_down2)

        tab_shade_top_colour = self._adjust_brightness(bg_colour, tab_shade_top)
        tab_shade_bottom_colour = self._adjust_brightness(bg_colour, tab_shade_bottom)

        text_bg_colour = self._get_colour_from_str(
                style_mgr.get_style_param('text_bg_colour'))
        text_fg_colour = self._get_colour_from_str(
                style_mgr.get_style_param('text_fg_colour'))
        text_selected_bg_colour = self._get_colour_from_str(
                style_mgr.get_style_param('text_selected_bg_colour'))
        text_selected_fg_colour = self._get_colour_from_str(
                style_mgr.get_style_param('text_selected_fg_colour'))
        text_disabled_fg_colour = self._get_colour_from_str(
                style_mgr.get_style_param('text_disabled_fg_colour'))

        def make_light(colour):
            return self._adjust_brightness(colour, contrast)

        def make_grad(colour):
            return self._adjust_brightness(colour, grad)

        def make_dark(colour):
            return self._adjust_brightness(colour, -contrast)

        # Get derived colours
        colours = {
            'icons_path'                  : icons_path,
            'bg_colour'                   : bg_colour,
            'bg_colour_light'             : make_light(bg_colour),
            'bg_colour_dark'              : make_dark(bg_colour),
            'fg_colour'                   : fg_colour,
            'disabled_fg_colour'          : disabled_fg_colour,
            'button_bg_colour'            : button_bg_colour,
            'button_bg_colour_light'      : make_light(button_bg_colour),
            'button_bg_colour_grad'       : make_grad(button_bg_colour),
            'button_bg_colour_dark'       : make_dark(button_bg_colour),
            'button_down_bg_colour'       : button_down_bg_colour,
            'button_down_bg_colour_light' : make_light(button_down_bg_colour),
            'button_down_bg_colour_grad'  : make_grad(button_down_bg_colour),
            'button_down_bg_colour_dark'  : make_dark(button_down_bg_colour),
            'button_down2_bg_colour'      : button_down2_bg_colour,
            'button_down2_bg_colour_light': make_light(button_down2_bg_colour),
            'button_down2_bg_colour_grad' : make_grad(button_down2_bg_colour),
            'button_down2_bg_colour_dark' : make_dark(button_down2_bg_colour),
            'button_fg_colour'            : button_fg_colour,
            'button_down_fg_colour'       : button_down_fg_colour,
            'button_down2_fg_colour'      : button_down2_fg_colour,
            'important_button_bg_colour'            : ib_bg_colour,
            'important_button_bg_colour_light'      : make_light(ib_bg_colour),
            'important_button_bg_colour_grad'       : make_grad(ib_bg_colour),
            'important_button_bg_colour_dark'       : make_dark(ib_bg_colour),
            'important_button_down_bg_colour'       : ib_down_bg_colour,
            'important_button_down_bg_colour_light' : make_light(ib_down_bg_colour),
            'important_button_down_bg_colour_grad'  : make_grad(ib_down_bg_colour),
            'important_button_down_bg_colour_dark'  : make_dark(ib_down_bg_colour),
            'important_button_down2_bg_colour'      : ib_down2_bg_colour,
            'important_button_down2_bg_colour_light': make_light(ib_down2_bg_colour),
            'important_button_down2_bg_colour_grad' : make_grad(ib_down2_bg_colour),
            'important_button_down2_bg_colour_dark' : make_dark(ib_down2_bg_colour),
            'important_button_fg_colour'            : ib_fg_colour,
            'important_button_down_fg_colour'       : ib_down_fg_colour,
            'important_button_down2_fg_colour'      : ib_down2_fg_colour,
            'scrollbar_bg_colour'         : bg_sunken_colour,
            'tab_shade_top_colour'        : tab_shade_top_colour,
            'tab_shade_top_colour_light'  : make_light(tab_shade_top_colour),
            'tab_shade_top_colour_dark'   : make_dark(tab_shade_top_colour),
            'tab_shade_bottom_colour'     : tab_shade_bottom_colour,
            'text_bg_colour'              : text_bg_colour,
            'text_fg_colour'              : text_fg_colour,
            'text_selected_bg_colour'     : text_selected_bg_colour,
            'text_selected_fg_colour'     : text_selected_fg_colour,
            'text_disabled_fg_colour'     : text_disabled_fg_colour,
        }

        template = style_mgr.get_style_sheet_template()

        replacements = {
            '<def_font_size>': '{}pt'.format(def_font_size),
            '<def_font_family>': def_font_family,
        }
        replacements.update({
                '<' + k + '>': (self._get_str_from_colour(v) if type(v) == tuple else v)
                for (k, v) in colours.items() })
        regexp = re.compile('|'.join(re.escape(k) for k in replacements.keys()))
        style_sheet = regexp.sub(lambda match: replacements[match.group(0)], template)

        return style_sheet


