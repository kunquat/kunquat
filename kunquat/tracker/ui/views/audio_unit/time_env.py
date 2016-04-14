# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
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

from kunquat.tracker.ui.views.envelope import Envelope
from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.numberslider import NumberSlider


class TimeEnvelope(QWidget):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._au_id = None
        self._updater = None

        header = HeaderLine(self._get_title())

        self._enabled_toggle = QCheckBox('Enabled')
        if self._allow_loop():
            self._loop_toggle = QCheckBox('Loop')
        if self._allow_release_toggle():
            self._release_toggle = QCheckBox('Release')
        self._scale_amount = NumberSlider(2, -4, 4, title='Scale amount:')
        self._scale_center = NumberSlider(0, -3600, 3600, title='Scale center:')

        h = QHBoxLayout()
        h.addWidget(self._enabled_toggle)
        if self._allow_loop():
            h.addWidget(self._loop_toggle)
        if self._allow_release_toggle():
            h.addWidget(self._release_toggle)
        h.addWidget(self._scale_amount)
        h.addWidget(self._scale_center)

        self._envelope = self._make_envelope_widget()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(header)
        v.addLayout(h)
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
        if self._allow_loop():
            QObject.connect(
                    self._loop_toggle,
                    SIGNAL('stateChanged(int)'),
                    self._loop_enabled_changed)
        if self._allow_release_toggle():
            QObject.connect(
                    self._release_toggle,
                    SIGNAL('stateChanged(int)'),
                    self._release_changed)
        QObject.connect(
                self._scale_amount,
                SIGNAL('numberChanged(float)'),
                self._scale_amount_changed)
        QObject.connect(
                self._scale_center,
                SIGNAL('numberChanged(float)'),
                self._scale_center_changed)
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
        self._scale_amount.setEnabled(self._get_enabled())
        self._scale_center.setEnabled(self._get_enabled())

        if self._allow_loop():
            old_block = self._loop_toggle.blockSignals(True)
            self._loop_toggle.setEnabled(self._get_enabled())
            self._loop_toggle.setCheckState(
                    Qt.Checked if self._get_loop_enabled() else Qt.Unchecked)
            self._loop_toggle.blockSignals(old_block)

        if self._allow_release_toggle():
            old_block = self._release_toggle.blockSignals(True)
            self._release_toggle.setEnabled(self._get_enabled())
            self._release_toggle.setCheckState(
                    Qt.Checked if self._get_release_enabled() else Qt.Unchecked)
            self._release_toggle.blockSignals(old_block)

        self._scale_amount.set_number(self._get_scale_amount())
        self._scale_center.set_number(self._get_scale_center())

        envelope = self._get_envelope_data()
        self._envelope.set_nodes(envelope['nodes'])
        if self._allow_loop():
            self._envelope.set_loop_markers(envelope['marks'])
            self._envelope.set_loop_enabled(self._get_loop_enabled())

    def _enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        self._set_enabled(new_enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _loop_enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        self._set_loop_enabled(new_enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _release_changed(self, state):
        new_enabled = (state == Qt.Checked)
        self._set_release_enabled(new_enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _scale_amount_changed(self, num):
        self._set_scale_amount(num)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _scale_center_changed(self, num):
        self._set_scale_center(num)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _envelope_changed(self):
        new_nodes, new_loop = self._envelope.get_clear_changed()

        envelope = self._get_envelope_data()
        if new_nodes:
            envelope['nodes'] = new_nodes
        if new_loop:
            envelope['marks'] = new_loop

        if new_nodes or new_loop:
            self._set_enabled(True)

        self._set_envelope_data(envelope)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    # Protected callbacks

    def _get_title(self):
        raise NotImplementedError

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

    def _get_scale_amount(self):
        raise NotImplementedError

    def _set_scale_amount(self, value):
        raise NotImplementedError

    def _get_scale_center(self):
        raise NotImplementedError

    def _set_scale_center(self, value):
        raise NotImplementedError

    def _get_envelope_data(self):
        raise NotImplementedError

    def _set_envelope_data(self, envelope):
        raise NotImplementedError


