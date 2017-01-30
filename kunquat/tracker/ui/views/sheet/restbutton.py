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

from kunquat.tracker.ui.model.trigger import Trigger
from kunquat.tracker.ui.views.updatingview import UpdatingView


class RestButton(QPushButton, UpdatingView):

    def __init__(self):
        super().__init__()
        self._sheet_manager = None

        self.setFlat(True)
        #self.setText('══')
        self.setToolTip('Add rest (1)')

    def _on_setup(self):
        self.register_action('signal_module', self._update_enabled)
        self.register_action('signal_edit_mode', self._update_enabled)
        self.register_action('signal_play', self._update_enabled)
        self.register_action('signal_silence', self._update_enabled)

        self._sheet_manager = self._ui_model.get_sheet_manager()

        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_icon_path('rest')
        icon = QIcon(icon_path)
        self.setIcon(icon)

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def _update_enabled(self):
        if (not self._sheet_manager.is_editing_enabled() or
                not self._sheet_manager.allow_editing()):
            self.setEnabled(False)
            return

        selection = self._ui_model.get_selection()
        location = selection.get_location()
        cur_column = self._sheet_manager.get_column_at_location(location)
        is_enabled = bool(cur_column)

        self.setEnabled(is_enabled)

    def _clicked(self):
        trigger = Trigger('n-', None)
        self._sheet_manager.add_trigger(trigger)


