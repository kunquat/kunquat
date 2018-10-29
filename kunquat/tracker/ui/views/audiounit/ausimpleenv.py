# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.headerline import HeaderLine
from .audiounitupdater import AudioUnitUpdater


class AudioUnitSimpleEnvelope(QWidget, AudioUnitUpdater):

    def __init__(self):
        super().__init__()
        header = HeaderLine(self._get_title())

        self._enabled_toggle = QCheckBox('Enabled')

        self._envelope = self._make_envelope_widget()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(header)
        v.addWidget(self._enabled_toggle)
        v.addWidget(self._envelope)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_au', self._update_envelope)
        self.register_action(self._get_update_signal_type(), self._update_envelope)
        self.register_action('signal_style_changed', self._update_style)

        self._envelope.set_ui_model(self._ui_model)

        self._enabled_toggle.stateChanged.connect(self._enabled_changed)
        self._envelope.get_envelope_view().envelopeChanged.connect(
                self._envelope_changed)

        self._update_envelope()
        self._update_style()

    def _on_teardown(self):
        self._envelope.unregister_updaters()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))
        self._envelope.update_style(style_mgr)

    def _update_envelope(self):
        old_block = self._enabled_toggle.blockSignals(True)
        self._enabled_toggle.setCheckState(
                Qt.Checked if self._get_enabled() else Qt.Unchecked)
        self._enabled_toggle.blockSignals(old_block)

        self._envelope.setEnabled(self._get_enabled())

        envelope = self._get_envelope_data()
        ev = self._envelope.get_envelope_view()
        ev.set_nodes(envelope['nodes'])

    def _enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        self._set_enabled(new_enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _envelope_changed(self):
        new_nodes, _ = self._envelope.get_envelope_view().get_clear_changed()

        envelope = self._get_envelope_data()
        if new_nodes:
            envelope['nodes'] = new_nodes

        if new_nodes:
            self._set_enabled(True)

        self._set_envelope_data(envelope)
        self._updater.signal_update(self._get_update_signal_type())

    # Protected interface

    def _get_update_signal_type(self):
        raise NotImplementedError

    def _get_title(self):
        raise NotImplementedError

    def _make_envelope_widget(self):
        raise NotImplementedError

    def _get_enabled(self):
        raise NotImplementedError

    def _set_enabled(self, enabled):
        raise NotImplementedError

    def _get_envelope_data(self):
        raise NotImplementedError

    def _set_envelope_data(self, envelope):
        raise NotImplementedError


