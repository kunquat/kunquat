# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
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


class ReplaceButton(QToolButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._sheet_manager = None

        self.setCheckable(True)
        self.setAutoRaise(True)
        self.setText('Replace')
        self.setToolTip('Replace (Insert)')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._sheet_manager = ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('replace')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_replace_mode' in signals:
            self._update_checked()

    def _update_checked(self):
        old_block = self.blockSignals(True)
        is_checked = self._sheet_manager.get_replace_mode()
        self.setChecked(is_checked)
        self.blockSignals(old_block)

    def _clicked(self):
        self._sheet_manager.set_replace_mode(self.isChecked())


