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

from kunquat.tracker.ui.views.updatingview import UpdatingView


class ReplaceButton(QPushButton, UpdatingView):

    def __init__(self):
        super().__init__()
        self._sheet_manager = None

        self.setCheckable(True)
        self.setFlat(True)
        #self.setText('Replace')
        self.setToolTip('Replace (Insert)')

    def _on_setup(self):
        self.register_action('signal_replace_mode', self._update_state)
        self.register_action('signal_play', self._update_state)
        self.register_action('signal_silence', self._update_state)
        self.register_action('signal_record_mode', self._update_state)

        self._sheet_manager = self._ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('replace')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def _update_state(self):
        old_block = self.blockSignals(True)
        disable = not self._sheet_manager.allow_editing()
        is_checked = self._sheet_manager.get_replace_mode() and not disable
        self.setChecked(is_checked)
        self.setEnabled(not disable)
        self.blockSignals(old_block)

    def _clicked(self):
        self._sheet_manager.set_replace_mode(self.isChecked())


