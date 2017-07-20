# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *


class HitMapToggle(QCheckBox):

    def __init__(self):
        super().__init__('Use hit keymap')
        self._ui_model = None
        self._updater = None

        self.setToolTip('Use hit keymap (Ctrl + H)')
        self.setFocusPolicy(Qt.NoFocus)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('stateChanged(int)'), self._set_hit_map_active)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        keymap_manager = self._ui_model.get_keymap_manager()
        is_active = keymap_manager.is_hit_keymap_active()
        is_checked = (self.checkState() == Qt.Checked)
        if is_checked != is_active:
            old_block = self.blockSignals(True)
            self.setCheckState(Qt.Checked if is_active else Qt.Unchecked)
            self.blockSignals(old_block)

    def _set_hit_map_active(self, state):
        active = (state == Qt.Checked)
        keymap_manager = self._ui_model.get_keymap_manager()
        keymap_manager.set_hit_keymap_active(active)
        self._updater.signal_update('signal_select_keymap')


