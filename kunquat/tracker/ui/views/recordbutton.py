# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2016
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


class RecordButton(QToolButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._sheet_manager = None
        self._playback_manager = None

        self.setCheckable(True)
        self.setText('Record')
        self.setAutoRaise(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._sheet_manager = ui_model.get_sheet_manager()
        self._playback_manager = ui_model.get_playback_manager()
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('record')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        QObject.connect(self, SIGNAL('clicked()'),
                        self._clicked)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_record_mode' in signals:
            self._update_checked()

    def _update_checked(self):
        old_block = self.blockSignals(True)
        is_checked = self._playback_manager.is_recording()
        self.setChecked(is_checked)
        self.blockSignals(old_block)

    def _clicked(self):
        if self._playback_manager.is_recording():
            self._playback_manager.stop_recording()
        else:
            self._playback_manager.start_recording()
            self._sheet_manager.set_typewriter_connected(True)
            self._ui_model.play()


