# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014
#          Toni Ruottu, Finland 2014
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

class CompositionToggle(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None
        self._up = None

        self.setText('Composition')
        self.setCheckable(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = self._ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def _perform_updates(self, signals):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_update = visibility_manager.get_visible()
        new_visible = UI_COMPOSITION in visibility_update
        if self.isChecked() != new_visible:
             self.setChecked(new_visible)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _clicked(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_update = visibility_manager.get_visible()
        if UI_COMPOSITION in visibility_update:
            visibility_manager.hide_composition()
        else:
            visibility_manager.show_composition()

