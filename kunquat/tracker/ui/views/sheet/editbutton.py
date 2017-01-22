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


class EditButton(QPushButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._sheet_manager = None

        self.setCheckable(True)
        self.setFlat(True)
        #self.setText('Edit')
        self.setToolTip('Edit (Space)')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._sheet_manager = ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('edit')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_edit_mode', 'signal_play', 'signal_silence', 'signal_record_mode'])
        if not signals.isdisjoint(update_signals):
            self._update_state()

    def _update_state(self):
        old_block = self.blockSignals(True)
        disable = not self._sheet_manager.allow_editing()
        is_checked = self._sheet_manager.get_typewriter_connected() and not disable
        self.setChecked(is_checked)
        self.setEnabled(not disable)
        self.blockSignals(old_block)

    def _clicked(self):
        self._sheet_manager.set_typewriter_connected(self.isChecked())


