# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.tracker.ui.views.updater import Updater


class ZoomButton(QPushButton, Updater):

    INFO = {
            'in': ('Zoom In', 'zoom_in', 'Ctrl + +'),
            'out': ('Zoom Out', 'zoom_out', 'Ctrl + -'),
            'original': ('Zoom to Original', 'zoom_reset', 'Ctrl + 0'),
            'expand_w': ('Expand Columns', 'col_expand', 'Ctrl + Alt + +'),
            'shrink_w': ('Shrink Columns', 'col_shrink', 'Ctrl + Alt + -'),
            'original_w': ('Reset Column Width', 'col_reset_width', 'Ctrl + Alt + 0'),
        }

    def __init__(self, mode):
        super().__init__()
        self._sheet_manager = None

        self._mode = mode
        self.setFlat(True)
        #self.setText(self._get_text(mode))
        self.setToolTip(self._get_tooltip(mode))

    def _on_setup(self):
        self.register_action('signal_sheet_zoom', self._update_enabled)
        self.register_action('signal_sheet_zoom_range', self._update_enabled)
        self.register_action('signal_sheet_column_width', self._update_enabled)

        self._sheet_manager = self._ui_model.get_sheet_manager()

        icon = self._get_icon(self._mode)
        self.setIcon(icon)

        self._update_enabled()
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def _update_enabled(self):
        zoom = self._sheet_manager.get_zoom()
        width = self._sheet_manager.get_column_width()
        if self._mode == 'in':
            _, maximum = self._sheet_manager.get_zoom_range()
            is_enabled = zoom < maximum
        elif self._mode == 'out':
            minimum, _ = self._sheet_manager.get_zoom_range()
            is_enabled = zoom > minimum
        elif self._mode == 'original':
            is_enabled = zoom != 0
        elif self._mode == 'expand_w':
            _, maximum = self._sheet_manager.get_column_width_range()
            is_enabled = width < maximum
        elif self._mode == 'shrink_w':
            minimum, _ = self._sheet_manager.get_column_width_range()
            is_enabled = width > minimum
        elif self._mode == 'original_w':
            is_enabled = width != 0

        self.setEnabled(is_enabled)

    def _get_text(self, mode):
        return ZoomButton.INFO[mode][0]

    def _get_icon(self, mode):
        icon_name = ZoomButton.INFO[mode][1]
        icon_bank = self._ui_model.get_icon_bank()
        try:
            icon_path = icon_bank.get_icon_path(icon_name)
        except ValueError:
            return QIcon()
        icon = QIcon(icon_path)
        return icon

    def _get_shortcut(self, mode):
        return ZoomButton.INFO[mode][2]

    def _get_tooltip(self, mode):
        return '{} ({})'.format(self._get_text(mode), self._get_shortcut(mode))

    def _clicked(self):
        if self._mode in ('in', 'out', 'original'):
            new_zoom = 0
            if self._mode == 'in':
                new_zoom = self._sheet_manager.get_zoom() + 1
            elif self._mode == 'out':
                new_zoom = self._sheet_manager.get_zoom() - 1
            self._sheet_manager.set_zoom(new_zoom)
        else:
            new_width = 0
            if self._mode == 'expand_w':
                new_width = self._sheet_manager.get_column_width() + 1
            elif self._mode == 'shrink_w':
                new_width = self._sheet_manager.get_column_width() - 1
            self._sheet_manager.set_column_width(new_width)


