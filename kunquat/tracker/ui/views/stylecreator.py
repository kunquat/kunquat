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
            app.setStyleSheet(style_manager.get_default_style_sheet())
            return

        icon_bank = self._ui_model.get_icon_bank()

        bg_colour_str = style_manager.get_style_param('bg_colour', '#555')
        fg_colour_str = style_manager.get_style_param('fg_colour', '#fed')
        disabled_fg_colour_str = style_manager.get_style_param(
                'disabled_fg_colour', '#999')

        bg_colour = self._get_colour_from_str(bg_colour_str)

        contrast = 0.3

        bg_colour_light = self._adjust_brightness(bg_colour, contrast)
        bg_colour_dark = self._adjust_brightness(bg_colour, -contrast)

        scrollbar_bg_colour = self._adjust_brightness(bg_colour, -0.2)

        template = '''
            QWidget
            {
                background-color: <bg_colour>;
                color: <fg_colour>;
            }

            QWidget:disabled
            {
                color: <disabled_fg_colour>;
            }

            QHeaderView::section
            {
                background-color: transparent;
            }

            QHeaderView::section:horizontal
            {
                border-top: none;
                border-left: 1px solid <bg_colour_light>;
                border-right: 1px solid <bg_colour_dark>;
                border-bottom: none;
            }

            QHeaderView::section:horizontal:first,
            QHeaderView::section:horizontal:only-one
            {
                border-left: none;
            }

            QHeaderView::section:vertical
            {
                border-top: 1px solid <bg_colour_light>;
                border-left: none;
                border-right: none;
                border-bottom: 1px solid <bg_colour_dark>;
                min-width: 3px;
            }

            QHeaderView::section:vertical:first,
            QHeaderView::section:vertical:only-one
            {
                border-top: none;
            }

            QLabel
            {
                background-color: transparent;
            }

            QScrollBar
            {
                border: none;
                padding: 0;
                background-color: <scrollbar_bg_colour>;
            }

            QScrollBar:horizontal
            {
                margin: 0 4px 0 4px;
                height: 14px;
            }

            QScrollBar:vertical
            {
                margin: 4px 0 4px 0;
                width: 14px;
            }

            QScrollBar::add-page, QScrollBar::sub-page
            {
                background: transparent;
                margin: 0;
                padding: 0;
            }

            QScrollBar::handle
            {
                border: 1px solid;
                border-top-color: <bg_colour_light>;
                border-left-color: <bg_colour_light>;
                border-right-color: <bg_colour_dark>;
                border-bottom-color: <bg_colour_dark>;
                padding: 0;
                background: <bg_colour>;
            }

            QScrollBar::handle:horizontal
            {
                margin: 0 10px 0 10px;
                min-width: 20px;
            }

            QScrollBar::handle:vertical
            {
                margin: 10px 0 10px 0;
                min-height: 20px;
            }

            QScrollBar::add-line, QScrollBar::sub-line
            {
                margin: 0;
                border: 1px solid;
                border-top-color: <bg_colour_light>;
                border-left-color: <bg_colour_light>;
                border-right-color: <bg_colour_dark>;
                border-bottom-color: <bg_colour_dark>;
                padding: 0;
                background: <bg_colour>;
                width: 12px;
                height: 12px;
            }

            QScrollBar::add-line:horizontal
            {
                border-top-left-radius: 0;
                border-top-right-radius: 2px;
                border-bottom-left-radius: 0;
                border-bottom-right-radius: 2px;
                subcontrol-position: right;
                subcontrol-origin: margin;
            }

            QScrollBar::add-line:vertical
            {
                border-top-left-radius: 0;
                border-top-right-radius: 0;
                border-bottom-left-radius: 2px;
                border-bottom-right-radius: 2px;
                subcontrol-position: bottom;
                subcontrol-origin: margin;
            }

            QScrollBar::sub-line:horizontal
            {
                border-top-left-radius: 2px;
                border-top-right-radius: 0;
                border-bottom-left-radius: 2px;
                border-bottom-right-radius: 0;
                subcontrol-position: left;
                subcontrol-origin: margin;
            }

            QScrollBar::sub-line:vertical
            {
                border-top-left-radius: 2px;
                border-top-right-radius: 2px;
                border-bottom-left-radius: 0;
                border-bottom-right-radius: 0;
                subcontrol-position: top;
                subcontrol-origin: margin;
            }

            QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical
            {
                border: 2px solid grey;
                width: 3px;
                height: 3px;
                background: white;
            }

            QSplitter::handle
            {
                /*image: url(/home/tjo/code/kunquat/kunquat/share/kunquat/icons/arrow_down_small.png);*/
            }

            QTabWidget::pane
            {
                border: 1px solid;
                border-top-color: <bg_colour_light>;
                border-left-color: <bg_colour_light>;
                border-right-color: <bg_colour_dark>;
                border-bottom-color: <bg_colour_dark>;
            }

            QTabWidget::tab-bar
            {
                left: 3px;
            }

            QTabBar::tab
            {
                border: 1px solid;
                border-top-color: <bg_colour_light>;
                border-left-color: <bg_colour_light>;
                border-right-color: <bg_colour_dark>;
                border-bottom: none;
                min-width: 2em;
                margin: 0;
                padding: 3px;
            }

            QTabBar::tab:selected
            {
            }

            QTabBar::tab:!selected
            {
                margin-top: 3px;
            }

            QTableView QTableCornerButton::section
            {
                background-color: transparent;
                border: none;
            }
            '''

        bg_colour_light_str = self._get_str_from_colour(bg_colour_light)
        bg_colour_dark_str = self._get_str_from_colour(bg_colour_dark)
        scrollbar_bg_colour_str = self._get_str_from_colour(scrollbar_bg_colour)

        replacements = {
            '<bg_colour>'          : bg_colour_str,
            '<bg_colour_dark>'     : bg_colour_dark_str,
            '<bg_colour_light>'    : bg_colour_light_str,
            '<fg_colour>'          : fg_colour_str,
            '<disabled_fg_colour>' : disabled_fg_colour_str,
            '<scrollbar_bg_colour>': scrollbar_bg_colour_str,
        }
        regexp = re.compile('|'.join(re.escape(k) for k in replacements.keys()))
        style_sheet = regexp.sub(lambda match: replacements[match.group(0)], template)

        app.setStyleSheet(style_sheet)


