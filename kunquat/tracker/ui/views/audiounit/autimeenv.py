# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.envelope import Envelope
from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.numberslider import NumberSlider
from .audiounitupdater import AudioUnitUpdater


class AudioUnitTimeEnvelope(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()
        header = HeaderLine(self._get_title())

        if self._allow_toggle_enabled():
            self._enabled_toggle = QCheckBox('Enabled')
        if self._allow_loop():
            self._loop_toggle = QCheckBox('Loop')
        if self._allow_release_toggle():
            self._release_toggle = QCheckBox('Release')

        h = QHBoxLayout()
        if self._allow_toggle_enabled():
            h.addWidget(self._enabled_toggle)
        if self._allow_loop():
            h.addWidget(self._loop_toggle)
        if self._allow_release_toggle():
            h.addWidget(self._release_toggle)
        h.addStretch()

        self._envelope = self._make_envelope_widget()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(header)
        v.addLayout(h)
        v.addWidget(self._envelope)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_au', self._update_envelope)
        self.register_action(self._get_update_signal_type(), self._update_envelope)
        self.register_action('signal_style_changed', self._update_style)

        self._envelope.set_icon_bank(self._ui_model.get_icon_bank())

        if self._allow_toggle_enabled():
            self._enabled_toggle.stateChanged.connect(self._enabled_changed)
        if self._allow_loop():
            self._loop_toggle.stateChanged.connect(self._loop_enabled_changed)
        if self._allow_release_toggle():
            self._release_toggle.stateChanged.connect(self._release_changed)
        self._envelope.get_envelope_view().envelopeChanged.connect(
                self._envelope_changed)

        self._update_envelope()
        self._update_style()

    def _update_style(self):
        self._envelope.update_style(self._ui_model.get_style_manager())

    def _update_envelope(self):
        is_enabled = True
        if self._allow_toggle_enabled():
            old_block = self._enabled_toggle.blockSignals(True)
            self._enabled_toggle.setCheckState(
                    Qt.Checked if self._get_enabled() else Qt.Unchecked)
            self._enabled_toggle.blockSignals(old_block)

            is_enabled = self._get_enabled()

        self._envelope.setEnabled(is_enabled)

        if self._allow_loop():
            old_block = self._loop_toggle.blockSignals(True)
            self._loop_toggle.setEnabled(is_enabled)
            self._loop_toggle.setCheckState(
                    Qt.Checked if self._get_loop_enabled() else Qt.Unchecked)
            self._loop_toggle.blockSignals(old_block)

        if self._allow_release_toggle():
            old_block = self._release_toggle.blockSignals(True)
            self._release_toggle.setEnabled(is_enabled)
            self._release_toggle.setCheckState(
                    Qt.Checked if self._get_release_enabled() else Qt.Unchecked)
            self._release_toggle.blockSignals(old_block)

        envelope = self._get_envelope_data()
        ev = self._envelope.get_envelope_view()
        ev.set_nodes(envelope['nodes'])
        if self._allow_loop():
            ev.set_loop_markers(envelope['marks'])
            ev.set_loop_enabled(self._get_loop_enabled())

    def _enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        self._set_enabled(new_enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _loop_enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        self._set_loop_enabled(new_enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _release_changed(self, state):
        new_enabled = (state == Qt.Checked)
        self._set_release_enabled(new_enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _envelope_changed(self):
        new_nodes, new_loop = self._envelope.get_envelope_view().get_clear_changed()

        envelope = self._get_envelope_data()
        if new_nodes:
            envelope['nodes'] = new_nodes
        if new_loop:
            envelope['marks'] = new_loop

        if new_nodes or new_loop:
            if self._allow_toggle_enabled():
                self._set_enabled(True)

        self._set_envelope_data(envelope)
        self._updater.signal_update(self._get_update_signal_type())

    # Protected callbacks

    def _get_title(self):
        raise NotImplementedError

    def _allow_toggle_enabled(self):
        return True

    def _allow_loop(self):
        raise NotImplementedError

    def _allow_release_toggle(self):
        return False

    def _make_envelope_widget(self):
        raise NotImplementedError

    def _get_update_signal_type(self):
        raise NotImplementedError

    def _get_enabled(self):
        raise NotImplementedError

    def _set_enabled(self, enabled):
        raise NotImplementedError

    def _get_loop_enabled(self):
        raise NotImplementedError

    def _set_loop_enabled(self, enabled):
        raise NotImplementedError

    def _get_release_enabled(self):
        raise NotImplementedError

    def _set_release_enabled(self, enabled):
        raise NotImplementedError

    def _get_envelope_data(self):
        raise NotImplementedError

    def _set_envelope_data(self, envelope):
        raise NotImplementedError


