# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2016
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

from .octavebutton import OctaveButton


class OctaveSelector(QFrame):

    def __init__(self):
        super().__init__()
        self.setFocusPolicy(Qt.NoFocus)
        self._ui_model = None
        self._updater = None
        self._typewriter_manager = None

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(4)
        h.addStretch(1)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._typewriter_manager = ui_model.get_typewriter_manager()

        self._update_layout()

    def unregister_updaters(self):
        for i in range(self.layout().count() - 1):
            button = self.layout().itemAt(i).widget()
            button.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set(['signal_select_keymap', 'signal_notation'])
        if not signals.isdisjoint(update_signals):
            self._update_layout()

    def _update_layout(self):
        layout = self.layout()

        old_button_count = max(0, layout.count() - 1)
        new_button_count = self._typewriter_manager.get_octave_count()

        # Create new widgets
        for i in range(old_button_count, new_button_count):
            button = self._get_button(i)
            layout.insertWidget(i, button)

        # Make sure that correct widgets are shown
        for i in range(new_button_count):
            layout.itemAt(i).widget().show()
        for i in range(new_button_count, old_button_count):
            layout.itemAt(i).widget().hide()

    def _get_buttons(self):
        octave_count = self._typewriter_manager.get_octave_count()
        buttons = (self._get_button(i) for i in range(octave_count))
        return buttons

    def _get_button(self, octave_id):
        button = OctaveButton(octave_id)
        button.set_ui_model(self._ui_model)
        return button


