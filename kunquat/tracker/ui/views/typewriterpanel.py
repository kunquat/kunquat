# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2016
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

import kunquat.tracker.cmdline as cmdline
from .octaveselector import OctaveSelector
from .typewriter import Typewriter
from .notationselect import NotationSelect
from .profilecontrol import ProfileControl


class TypewriterPanel(QFrame):

    def __init__(self):
        QFrame.__init__(self)
        self._ui_model = None
        self._notation_select = NotationSelect()
        self._hit_map_toggle = HitMapToggle()
        self._octave_selector = OctaveSelector()
        self._typewriter = Typewriter()
        self._profile_control = ProfileControl()

        il = QHBoxLayout()
        il.setMargin(0)
        il.setSpacing(4)
        il.addWidget(self._notation_select)
        il.addWidget(self._hit_map_toggle)
        il.addStretch(1)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 0)
        v.setSpacing(2)
        v.addLayout(il)
        v.addWidget(self._octave_selector)
        v.addWidget(self._typewriter)
        self.setLayout(v)

        self._typewriter.setFocus()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._notation_select.set_ui_model(ui_model)
        self._hit_map_toggle.set_ui_model(ui_model)
        self._octave_selector.set_ui_model(ui_model)
        self._typewriter.set_ui_model(ui_model)
        self._profile_control.set_ui_model(ui_model)

    def keyPressEvent(self, event):
        modifiers = event.modifiers()
        key = event.key()
        if modifiers == Qt.ControlModifier and key == Qt.Key_P:
            if cmdline.get_experimental():
                self._profile_control.show()

    def unregister_updaters(self):
        self._typewriter.unregister_updaters()
        self._octave_selector.unregister_updaters()
        self._hit_map_toggle.unregister_updaters()
        self._notation_select.unregister_updaters()


class HitMapToggle(QCheckBox):

    def __init__(self):
        QCheckBox.__init__(self, 'Use hit keymap')
        self._ui_model = None
        self._updater = None

        self.setToolTip('Use hit keymap (Ctrl + H)')

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
        self._updater.signal_update(set(['signal_select_keymap']))


