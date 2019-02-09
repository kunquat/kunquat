# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.updater import Updater
from kunquat.tracker.ui.views.utils import get_abs_window_size
from .sheethelp import SheetHelp


class SheetHelpWindow(QWidget, Updater):

    def __init__(self):
        super().__init__()

        self._sheet_help = SheetHelp()
        self.add_to_updaters(self._sheet_help)

        self.setWindowTitle('Composition sheet instructions')

        v = QVBoxLayout()
        v.addWidget(self._sheet_help)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(0)

    def closeEvent(self, event):
        event.ignore()
        self._ui_model.get_visibility_manager().hide_sheet_help()

    def sizeHint(self):
        return get_abs_window_size(0.4, 0.7)


