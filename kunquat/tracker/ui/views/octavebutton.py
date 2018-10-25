# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2013-2014
#          Tomi Jylhä-Ollila, Finland 2013-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *


class OctaveButton(QPushButton):

    def __init__(self, octave_id):
        super().__init__()
        self.setFocusPolicy(Qt.NoFocus)
        self._octave_id = octave_id
        self._typewriter_mgr = None
        self._control_mgr = None
        self._updater = None

        self.setCheckable(True)
        self.setFixedWidth(60)
        self.setToolTip("Select octave ('<': previous, '>': next)")
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

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._control_mgr = ui_model.get_control_manager()
        self._typewriter_mgr = ui_model.get_typewriter_manager()
        self._button_model = self._typewriter_mgr.get_octave_button_model(
                self._octave_id)

        self._update_name()
        self._update_pressed()
        self._update_style()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

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
        self._led.setFixedWidth(style_mgr.get_scaled_size(1))

    def _perform_updates(self, signals):
        name_update_signals = set(['signal_notation', 'signal_select_keymap'])
        if not signals.isdisjoint(name_update_signals):
            self._update_name()

        if any(s in signals for s in ['signal_octave', 'signal_init']):
            self._update_pressed()

        if 'signal_style_changed' in signals:
            self._update_style()


