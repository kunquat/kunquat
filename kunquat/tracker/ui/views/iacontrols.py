# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import string

from PyQt4.QtCore import *
from PyQt4.QtGui import *


class IAControls(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._inf_toggle = InfiniteToggle()

        v = QVBoxLayout()
        v.setMargin(4)
        v.setSpacing(4)
        v.addWidget(self._inf_toggle)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._inf_toggle.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._inf_toggle.unregister_updaters()


class InfiniteToggle(QCheckBox):

    def __init__(self):
        QCheckBox.__init__(self)
        self._ui_model = None
        self._updater = None

        self.setText('Enable infinite mode')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('stateChanged(int)'), self._toggle_infinite_mode)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'infinite_mode' in signals:
            self._update_inf_setting()

    def _update_inf_setting(self):
        playback_manager = self._ui_model.get_playback_manager()
        old_block = self.blockSignals(True)
        self.setCheckState(
                Qt.Checked if playback_manager.get_infinite_mode() else Qt.Unchecked)
        self.blockSignals(old_block)

    def _toggle_infinite_mode(self, new_state):
        enabled = (new_state == Qt.Checked)

        playback_manager = self._ui_model.get_playback_manager()
        playback_manager.set_infinite_mode(enabled)

        self._updater.signal_update(set(['infinite_mode']))


