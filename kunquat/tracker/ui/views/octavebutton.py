# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2013-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.model.keymapmanager import KeyboardAction
from .updater import Updater


class OctaveButton(QPushButton, Updater):

    def __init__(self, octave_id):
        super().__init__()
        self.setFocusPolicy(Qt.NoFocus)
        self._octave_id = octave_id
        self._typewriter_mgr = None
        self._control_mgr = None
        self._updater = None

        self.setCheckable(True)
        self.setFixedWidth(60)
        self.setToolTip('Select octave')
        layout = QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        octavename = QLabel()
        self._octavename = octavename
        octavename.setAlignment(Qt.AlignCenter)
        layout.addWidget(octavename)
        led = QLabel()
        self._led = led
        self._led.setFixedWidth(10)
        layout.addWidget(led)
        layout.setAlignment(Qt.AlignCenter)

        self.clicked.connect(self._select_octave)

    def _select_octave(self):
        self._typewriter_mgr.set_octave(self._octave_id)

    def _on_setup(self):
        self._control_mgr = self._ui_model.get_control_manager()
        self._typewriter_mgr = self._ui_model.get_typewriter_manager()
        self._button_model = self._typewriter_mgr.get_octave_button_model(
                self._octave_id)

        self.register_action('signal_notation', self._update_name)
        self.register_action('signal_select_keymap', self._update_name)
        self.register_action('signal_octave', self._update_pressed)
        self.register_action('signal_init', self._update_pressed)
        self.register_action('signal_style_changed', self._update_style)

        keymap_mgr = self._ui_model.get_keymap_manager()
        lower_key_name = keymap_mgr.get_key_name(
                keymap_mgr.get_action_location(KeyboardAction.OCTAVE_DOWN))
        higher_key_name = keymap_mgr.get_key_name(
                keymap_mgr.get_action_location(KeyboardAction.OCTAVE_UP))
        if lower_key_name and higher_key_name:
            self.setToolTip('Select octave ({}: lower, {}: higher)'.format(
                lower_key_name, higher_key_name))

        self._update_name()
        self._update_pressed()
        self._update_style()

    def update_led_state(self, enabled):
        if enabled:
            self._led.setText('*')
        else:
            self._led.setText('')

    def _update_name(self):
        octave_name = self._button_model.get_name()
        self._octavename.setText(octave_name)

    def _update_pressed(self):
        old_block = self.blockSignals(True)
        if self._button_model.is_selected():
            self.setChecked(True)
        else:
            self.setChecked(False)
        self.blockSignals(old_block)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.setFixedWidth(style_mgr.get_scaled_size_param('typewriter_button_size'))
        self._led.setFixedWidth(style_mgr.get_scaled_size(1.5))


