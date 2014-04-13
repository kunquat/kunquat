# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
#          Toni Ruottu, Finland 2013-2014
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

from kunquat.tracker.ui.identifiers import *
from sheet.sheet import Sheet


class Composition(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._visible = None
        self._sheet = Sheet()

        v = QVBoxLayout()
        v.addWidget(self._sheet)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._sheet.set_ui_model(ui_model)
        self._updater = self._ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

    def _show(self):
        self.show()
        self._visible = True

    def _hide(self):
        self.hide()
        self._visible = False

    def _update_visibility(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_update = visibility_manager.get_visible()
        new_visible = UI_COMPOSITION in visibility_update
        if self._visible != new_visible:
             if new_visible:
                 self._show()
             else:
                 self._hide()

    def _perform_updates(self, signals):
        self._update_visibility()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._sheet.unregister_updaters()

