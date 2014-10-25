# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
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

from kunquat.tracker.ui.views.envelope import Envelope
from numberslider import NumberSlider


class ForceEnvelope(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._ins_id = None
        self._updater = None

        self._enabled_toggle = QCheckBox('Enabled')
        self._loop_toggle = QCheckBox('Loop')
        self._scale_amount = NumberSlider(2, -4, 4)

        h = QHBoxLayout()
        h.addWidget(self._enabled_toggle)
        h.addWidget(self._loop_toggle)
        h.addWidget(self._scale_amount)

        self._envelope = Envelope()
        self._envelope.set_node_count_max(32)
        self._envelope.set_y_range(0, 1)
        self._envelope.set_x_range(0, 4)
        self._envelope.set_first_lock(True, False)
        self._envelope.set_x_range_adjust(False, True)

        v = QVBoxLayout()
        v.addLayout(h)
        v.addWidget(self._envelope)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

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
                self._loop_toggle,
                SIGNAL('stateChanged(int)'),
                self._loop_enabled_changed)
        QObject.connect(
                self._scale_amount,
                SIGNAL('numberChanged(float)'),
                self._scale_amount_changed)
        QObject.connect(
                self._envelope,
                SIGNAL('envelopeChanged()'),
                self._envelope_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_instrument' in signals:
            self._update_envelope()

    def _update_envelope(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        envelope = instrument.get_force_envelope()

        old_block = self._enabled_toggle.blockSignals(True)
        self._enabled_toggle.setCheckState(
                Qt.Checked if envelope['enabled'] else Qt.Unchecked)
        self._enabled_toggle.blockSignals(old_block)

        old_block = self._loop_toggle.blockSignals(True)
        self._loop_toggle.setCheckState(
                Qt.Checked if envelope['loop'] else Qt.Unchecked)
        self._loop_toggle.blockSignals(old_block)

        self._scale_amount.set_number(envelope['scale_amount'])

        self._envelope.set_nodes(envelope['envelope']['nodes'])
        self._envelope.set_loop_markers(envelope['envelope']['marks'])
        self._envelope.set_loop_enabled(envelope['loop'])

    def _bool_enabled_changed(self, key, state):
        new_enabled = (state == Qt.Checked)

        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        envelope = instrument.get_force_envelope()

        envelope[key] = new_enabled

        instrument.set_force_envelope(envelope)
        self._updater.signal_update(set(['signal_instrument']))

    def _enabled_changed(self, state):
        self._bool_enabled_changed('enabled', self._enabled_toggle.checkState())

    def _loop_enabled_changed(self, state):
        self._bool_enabled_changed('loop', self._loop_toggle.checkState())

    def _scale_amount_changed(self, num):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        envelope = instrument.get_force_envelope()

        envelope['scale_amount'] = num

        instrument.set_force_envelope(envelope)
        self._updater.signal_update(set(['signal_instrument']))

    def _envelope_changed(self):
        new_nodes, new_loop = self._envelope.get_clear_changed()

        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        envelope = instrument.get_force_envelope()

        if new_nodes:
            envelope['envelope']['nodes'] = new_nodes
        if new_loop:
            envelope['envelope']['marks'] = new_loop

        instrument.set_force_envelope(envelope)
        self._updater.signal_update(set(['signal_instrument']))


