# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *


class ZoomButton(QToolButton):

    def __init__(self, mode):
        QToolButton.__init__(self)
        self._ui_model = None
        self._updater = None
        self._sheet_manager = None

        self._mode = mode
        self.setText(self._get_text(mode))
        self.setIcon(self._get_icon(mode))
        self.setAutoRaise(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._sheet_manager = ui_model.get_sheet_manager()

        self._update_enabled()
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_sheet_zoom' in signals:
            self._update_enabled()
        if 'signal_sheet_zoom_range' in signals:
            self._update_enabled()

    def _update_enabled(self):
        zoom = self._sheet_manager.get_zoom()
        if self._mode == 'in':
            _, maximum = self._sheet_manager.get_zoom_range()
            is_enabled = zoom < maximum
        elif self._mode == 'out':
            minimum, _ = self._sheet_manager.get_zoom_range()
            is_enabled = zoom > minimum
        else:
            is_enabled = zoom != 0

        self.setEnabled(is_enabled)

    def _get_text(self, mode):
        texts = {
                'in': 'Zoom In',
                'out': 'Zoom Out',
                'original': 'Zoom to Original',
            }
        return texts[mode]

    def _get_icon(self, mode):
        names = {
                'in': 'zoom-in',
                'out': 'zoom-out',
                'original': 'zoom-original',
            }
        return QIcon.fromTheme(names[mode])

    def _clicked(self):
        new_zoom = 0
        if self._mode == 'in':
            new_zoom = self._sheet_manager.get_zoom() + 1
        elif self._mode == 'out':
            new_zoom = self._sheet_manager.get_zoom() - 1

        self._sheet_manager.set_zoom(new_zoom)


