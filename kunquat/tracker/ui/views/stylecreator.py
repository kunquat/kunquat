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

        # Get colours from the configuration
        contrast = 0.3
        grad = -0.07
        button_down = -0.15

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

            QComboBox
            {
                border: 1px solid;
                border-top-color: <button_bg_colour_light>;
                border-left-color: <button_bg_colour_light>;
                border-right-color: <button_bg_colour_dark>;
                border-bottom-color: <button_bg_colour_dark>;
                border-radius: 2px;
                padding: 3px;
                background-color: qlineargradient(x1: 0, y1: 0, x2: 0.1, y2: 1,
                    stop: 0.5 <button_bg_colour>, stop: 1 <button_bg_colour_grad>);
                color: <button_fg_colour>;
            }

            QComboBox::drop-down
            {
                subcontrol-origin: margin;
                subcontrol-position: top right;
                width: 12px;
                margin: 0;
                border: 1px solid;
                border-top-color: <button_bg_colour_light>;
                border-left-color: <button_bg_colour_light>;
                border-right-color: <button_bg_colour_dark>;
                border-bottom-color: <button_bg_colour_dark>;
                border-radius: 0;
                border-top-right-radius: 2px;
                border-bottom-right-radius: 2px;
                padding: 3px;
                background-color: qlineargradient(x1: 0, y1: 0, x2: 0.1, y2: 1,
                    stop: 0.5 <button_bg_colour>, stop: 1 <button_bg_colour_grad>);
                color: <button_fg_colour>;
            }

            QComboBox:!editable, QComboBox::drop-down:editable
            {
                margin: 0;
                border: 1px solid;
                border-top-color: <button_bg_colour_light>;
                border-left-color: <button_bg_colour_light>;
                border-right-color: <button_bg_colour_dark>;
                border-bottom-color: <button_bg_colour_dark>;
                border-radius: 2px;
                padding: 3px;
                background-color: <button_bg_colour>;
                /*
                background-color: qlineargradient(x1: 0, y1: 0, x2: 0.1, y2: 1,
                    stop: 0.5 <button_bg_colour>, stop: 1 <button_bg_colour_grad>);
                */
                color: <button_fg_colour>;
            }

            QComboBox::down-arrow
            {
                /*image: url(/home/tjo/code/kunquat/kunquat/share/kunquat/icons/arrow_down_small.png);*/
            }

            QComboBox QAbstractItemView
            {
                border: 1px solid;
                border-top-color: <bg_colour_light>;
                border-left-color: <bg_colour_light>;
                border-right-color: <bg_colour_dark>;
                border-bottom-color: <bg_colour_dark>;
                border-radius: 2px;
                background-color: <bg_colour>;
                color: <fg_colour>;

                selection-background-color: qlineargradient(x1: 0, y1: 0, x2: 0.1, y2: 1,
                    stop: 0.5 <button_bg_colour>, stop: 1 <button_bg_colour_grad>);
            }

            QPushButton, QToolButton:hover
            {
                border: 1px solid;
                border-top-color: <button_bg_colour_light>;
                border-left-color: <button_bg_colour_light>;
                border-right-color: <button_bg_colour_dark>;
                border-bottom-color: <button_bg_colour_dark>;
                border-radius: 2px;
                padding: 3px;
                background-color: qlineargradient(x1: 0, y1: 0, x2: 0.1, y2: 1,
                    stop: 0.5 <button_bg_colour>, stop: 1 <button_bg_colour_grad>);
                color: <button_fg_colour>;
            }

            QPushButton:pressed, QToolButton:pressed
            {
                border: 1px solid;
                border-top-color: <button_down_bg_colour_light>;
                border-left-color: <button_down_bg_colour_light>;
                border-right-color: <button_down_bg_colour_dark>;
                border-bottom-color: <button_down_bg_colour_dark>;
                background-color: qlineargradient(x1: 0, y1: 0, x2: 0.1, y2: 1,
                    stop: 0.5 <button_down_bg_colour>,
                    stop: 1 <button_down_bg_colour_grad>);
                color: <button_fg_colour>;
            }

            QLineEdit, QSpinBox, QDoubleSpinBox
            {
                margin: 1px;
                border: 2px solid;
                border-top-color: <bg_colour_dark>;
                border-left-color: <bg_colour_dark>;
                border-right-color: <bg_colour_light>;
                border-bottom-color: <bg_colour_light>;
                border-radius: 3px;
                padding: 0;
                background-color: <text_bg_colour>;
                color: <text_fg_colour>;
            }

            QSpinBox, QDoubleSpinBox
            {
                padding-right: 16px;
                border-right-color: <text_bg_colour>;
            }

            QSpinBox::up-button, QDoubleSpinBox::up-button,
            QSpinBox::down-button, QDoubleSpinBox::down-button
            {
                width: 16px;
                border: 1px solid;
                border-top-color: <button_bg_colour_light>;
                border-left-color: <button_bg_colour_light>;
                border-right-color: <button_bg_colour_dark>;
                border-bottom-color: <button_bg_colour_dark>;
                padding: 0;
                background-color: <button_bg_colour>;
            }

            QSpinBox::up-button:pressed, QDoubleSpinBox::up-button:pressed,
            QSpinBox::down-button:pressed, QDoubleSpinBox::down-button:pressed
            {
                border-top-color: <button_down_bg_colour_light>;
                border-left-color: <button_down_bg_colour_light>;
                border-right-color: <button_down_bg_colour_dark>;
                border-bottom-color: <button_down_bg_colour_dark>;
                background-color: <button_down_bg_colour>;
            }

            QSpinBox::up-button, QDoubleSpinBox::up-button
            {
                subcontrol-origin: border;
                subcontrol-position: top right;
                border-top-right-radius: 2px;
            }

            QSpinBox::down-button, QDoubleSpinBox::down-button
            {
                subcontrol-origin: border;
                subcontrol-position: bottom right;
                border-bottom-right-radius: 2px;
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

            QTableView
            {
                margin: 0;
                border: 2px solid;
                border-top-color: <bg_colour_dark>;
                border-left-color: <bg_colour_dark>;
                border-right-color: <bg_colour_light>;
                border-bottom-color: <bg_colour_light>;
                border-radius: 3px;
                padding: 0;
                background-color: <text_bg_colour>;
                color: <text_fg_colour>;
                gridline-color: <bg_colour_dark>;
            }

            QTableView QTableCornerButton::section
            {
                background-color: transparent;
                border: none;
            }
            '''

        replacements = { '<' + k + '>': self._get_str_from_colour(v)
                for (k, v) in colours.items() }
        regexp = re.compile('|'.join(re.escape(k) for k in replacements.keys()))
        style_sheet = regexp.sub(lambda match: replacements[match.group(0)], template)

        app.setStyleSheet(style_sheet)


