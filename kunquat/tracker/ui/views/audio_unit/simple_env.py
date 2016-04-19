# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
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

from kunquat.tracker.ui.views.headerline import HeaderLine


class SimpleEnvelope(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._ui_model = None
        self._updater = None

        header = HeaderLine(self._get_title())

        self._enabled_toggle = QCheckBox('Enabled')

        self._envelope = self._make_envelope_widget()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(6)
        v.addWidget(header)
        v.addWidget(self._enabled_toggle)
        v.addWidget(self._envelope)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_envelope()

        QObject.connect(
                self._enabled_toggle,
                SIGNAL('stateChanged(int)'),
                self._enabled_changed)
        QObject.connect(
                self._envelope,
                SIGNAL('envelopeChanged()'),
                self._envelope_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_envelope()

    def _update_envelope(self):
        old_block = self._enabled_toggle.blockSignals(True)
        self._enabled_toggle.setCheckState(
                Qt.Checked if self._get_enabled() else Qt.Unchecked)
        self._enabled_toggle.blockSignals(old_block)

        self._envelope.setEnabled(self._get_enabled())

        envelope = self._get_envelope_data()
        self._envelope.set_nodes(envelope['nodes'])

    def _enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        self._set_enabled(new_enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _envelope_changed(self):
        new_nodes, _ = self._envelope.get_clear_changed()

        envelope = self._get_envelope_data()
        if new_nodes:
            envelope['nodes'] = new_nodes

        if new_nodes:
            self._set_enabled(True)

        self._set_envelope_data(envelope)
        self._updater.signal_update(set([self._get_update_signal_type()]))

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


