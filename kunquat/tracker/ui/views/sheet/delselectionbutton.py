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


class DelSelectionButton(QPushButton, Updater):

    def __init__(self):
        super().__init__()
        self._sheet_manager = None

        self.setFlat(True)
        #self.setText('Del')
        self.setToolTip('Delete selection (Delete)')

    def _on_setup(self):
        self.register_action('signal_selection', self._update_enabled)
        self.register_action('signal_module', self._update_enabled)
        self.register_action('signal_column', self._update_enabled)
        self.register_action('signal_edit_mode', self._update_enabled)

        self._sheet_manager = self._ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('delete')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        self._update_enabled()
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def _update_enabled(self):
        if not self._sheet_manager.is_editing_enabled():
            self.setEnabled(False)
            return

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        if not location:
            self.setEnabled(False)
            return

        cur_column = self._sheet_manager.get_column_at_location(location)

        has_trigger = bool(cur_column) and cur_column.has_trigger(
                location.get_row_ts(), location.get_trigger_index())

        self.setEnabled(has_trigger)

    def _clicked(self):
        self._sheet_manager.try_remove_trigger()


