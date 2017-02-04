# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2017
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

from .octavebutton import OctaveButton
from .updater import Updater


class OctaveSelector(QFrame, Updater):

    def __init__(self):
        super().__init__()
        self.setFocusPolicy(Qt.NoFocus)
        self._typewriter_manager = None

        self._title = QLabel('Octave:')

        self._button_layout = QHBoxLayout()
        self._button_layout.setContentsMargins(0, 0, 0, 0)
        self._button_layout.setSpacing(4)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._title)
        h.addLayout(self._button_layout)
        h.addStretch(1)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_select_keymap', self._update_layout)
        self.register_action('signal_notation', self._update_layout)

        self._typewriter_manager = self._ui_model.get_typewriter_manager()
        self._update_layout()

    def _update_layout(self):
        keymap_manager = self._ui_model.get_keymap_manager()
        use_hit_keymap = keymap_manager.is_hit_keymap_active()
        if use_hit_keymap:
            self._title.setText('Hit bank:')
        else:
            self._title.setText('Octave:')

        old_button_count = self._button_layout.count()
        new_button_count = self._typewriter_manager.get_octave_count()

        # Create new widgets
        for i in range(old_button_count, new_button_count):
            button = self._get_button(i)
            self._button_layout.insertWidget(i, button)

        # Make sure that correct widgets are shown
        for i in range(new_button_count):
            self._button_layout.itemAt(i).widget().show()
        for i in range(new_button_count, old_button_count):
            self._button_layout.itemAt(i).widget().hide()

    def _get_button(self, octave_id):
        button = OctaveButton(octave_id)
        self.add_to_updaters(button)
        return button


