# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .octavebutton import OctaveButton
from .updater import Updater


class OctaveSelector(QFrame, Updater):

    def __init__(self):
        super().__init__()
        self.setFocusPolicy(Qt.NoFocus)
        self._typewriter_mgr = None

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
        self.register_action('signal_change', self._update_leds)

        self._typewriter_mgr = self._ui_model.get_typewriter_manager()
        self._update_layout()

    def _update_leds(self):
        if not self.isVisible():
            return

        octaves_enabled = self._typewriter_mgr.get_enabled_octave_leds()
        for octave_id in range(self._button_layout.count()):
            button = self._button_layout.itemAt(octave_id).widget()
            button.update_led_state(octave_id in octaves_enabled)

    def _update_layout(self):
        style_mgr = self._ui_model.get_style_manager()
        keymap_mgr = self._ui_model.get_keymap_manager()

        use_hit_keymap = keymap_mgr.is_hit_keymap_active()
        if use_hit_keymap:
            self._title.setText('Hit bank:')
        else:
            self._title.setText('Octave:')

        spacing = style_mgr.get_scaled_size_param('medium_padding')
        self._button_layout.setSpacing(spacing)
        self.layout().setSpacing(spacing)

        old_button_count = self._button_layout.count()
        new_button_count = self._typewriter_mgr.get_octave_count()

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


