# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2017
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

from .updatingview import UpdatingView


class RecordButton(QToolButton, UpdatingView):

    def __init__(self):
        super().__init__()
        self._sheet_manager = None
        self._playback_manager = None

        self.setCheckable(True)
        self.setText('Record')
        self.setAutoRaise(True)

    def _on_setup(self):
        self.register_action('signal_record_mode', self._update_checked)

        self._sheet_manager = self._ui_model.get_sheet_manager()
        self._playback_manager = self._ui_model.get_playback_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('record')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        QObject.connect(self, SIGNAL('clicked()'),
                        self._clicked)

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


