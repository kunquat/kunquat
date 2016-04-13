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

from PyQt4.QtCore import *
from PyQt4.QtGui import *


class DelSelectionButton(QToolButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None
        self._sheet_manager = None

        self.setAutoRaise(True)
        self.setText('Del')
        self.setToolTip('Delete selection (Delete)')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._sheet_manager = ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('delete')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        self._update_enabled()
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set([
            'signal_selection', 'signal_module', 'signal_column', 'signal_edit_mode'])
        if not signals.isdisjoint(update_signals):
            self._update_enabled()

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


