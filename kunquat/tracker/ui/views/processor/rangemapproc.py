# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2020
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
from kunquat.tracker.ui.views.varprecspinbox import VarPrecSpinBox
from . import utils
from .procsimpleenv import ProcessorSimpleEnvelope
from .processorupdater import ProcessorUpdater


class RangeMapProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Range map'

    def __init__(self):
        super().__init__()
        self._from_min = RangeValue()
        self._from_max = RangeValue()
        self._min_to = RangeValue()
        self._max_to = RangeValue()
        self._clamp_dest_min = ClampToggle('Clamp to minimum destination')
        self._clamp_dest_max = ClampToggle('Clamp to maximum destination')
        self._envelope = MappingEnvelope()

        self.add_to_updaters(self._envelope)

        self._map_layout = QGridLayout()
        self._map_layout.setContentsMargins(0, 0, 0, 0)
        self._map_layout.setVerticalSpacing(0)
        self._map_layout.addWidget(QLabel('Source range minimum:'), 0, 0)
        self._map_layout.addWidget(self._from_min, 0, 1)
        self._map_layout.addWidget(QLabel('Source range maximum:'), 0, 2)
        self._map_layout.addWidget(self._from_max, 0, 3)
        self._map_layout.addWidget(QWidget(), 0, 4)
        self._map_layout.addWidget(QLabel('Map minimum to:'), 1, 0)
        self._map_layout.addWidget(self._min_to, 1, 1)
        self._map_layout.addWidget(QLabel('Map maximum to:'), 1, 2)
        self._map_layout.addWidget(self._max_to, 1, 3)

        v = QVBoxLayout()
        v.addLayout(self._map_layout)
        v.addWidget(self._clamp_dest_min)
        v.addWidget(self._clamp_dest_max)
        v.addWidget(self._envelope)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_all)
        self.register_action('signal_style_changed', self._update_style)

        self._from_min.editingFinished.connect(
                self._set_from_min, type=Qt.QueuedConnection)
        self._from_max.editingFinished.connect(
                self._set_from_max, type=Qt.QueuedConnection)
        self._min_to.valueChanged.connect(self._set_min_to)
        self._max_to.valueChanged.connect(self._set_max_to)
        self._clamp_dest_min.stateChanged.connect(self._set_clamp_dest_min)
        self._clamp_dest_max.stateChanged.connect(self._set_clamp_dest_max)

        self._update_style()
        self._update_all()

    def _get_update_signal_type(self):
        return 'signal_rangemap_{}'.format(self._proc_id)

    def _get_range_map_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        for vps in (self._from_min, self._from_max, self._min_to, self._max_to):
            vps.update_style(style_mgr)

        self._map_layout.setHorizontalSpacing(
                style_mgr.get_scaled_size_param('large_padding'))

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

    def _update_all(self):
        params = self._get_range_map_params()

        self._from_min.set_value(params.get_from_min())
        self._from_max.set_value(params.get_from_max())
        self._min_to.set_value(params.get_min_to())
        self._max_to.set_value(params.get_max_to())

        self._clamp_dest_min.set_enabled(params.get_clamp_dest_min())
        self._clamp_dest_max.set_enabled(params.get_clamp_dest_max())

    def _set_from_min(self):
        params = self._get_range_map_params()
        params.set_from_min(self._from_min.value())
        params.set_from_max(max(params.get_from_min(), params.get_from_max()))
        self._updater.signal_update(self._get_update_signal_type())

    def _set_from_max(self):
        params = self._get_range_map_params()
        params.set_from_max(self._from_max.value())
        params.set_from_min(min(params.get_from_min(), params.get_from_max()))
        self._updater.signal_update(self._get_update_signal_type())

    def _set_min_to(self, value):
        self._get_range_map_params().set_min_to(value)
        self._updater.signal_update(self._get_update_signal_type())

    def _set_max_to(self, value):
        self._get_range_map_params().set_max_to(value)
        self._updater.signal_update(self._get_update_signal_type())

    def _set_clamp_dest_min(self, state):
        enabled = (state == Qt.Checked)
        self._get_range_map_params().set_clamp_dest_min(enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _set_clamp_dest_max(self, state):
        enabled = (state == Qt.Checked)
        self._get_range_map_params().set_clamp_dest_max(enabled)
        self._updater.signal_update(self._get_update_signal_type())


class RangeValue(VarPrecSpinBox):

    def __init__(self):
        super().__init__(step_decimals=0, max_decimals=6)
        self.setRange(-99999, 99999)

    def set_value(self, value):
        if self.value() != value:
            old_block = self.blockSignals(True)
            self.setValue(value)
            self.blockSignals(old_block)


class ClampToggle(QCheckBox):

    def __init__(self, label):
        super().__init__(label)

    def set_enabled(self, enabled):
        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self.blockSignals(old_block)


class MappingEnvelope(ProcessorSimpleEnvelope):

    def __init__(self):
        super().__init__()

    def _get_range_map_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _make_envelope_widget(self):
        envelope = Envelope()
        ev = envelope.get_envelope_view()
        ev.set_node_count_max(32)
        ev.set_y_range(0, 1)
        ev.set_x_range(0, 1)
        ev.set_first_lock(True, False)
        ev.set_last_lock(True, False)

        return envelope

    def _get_update_signal_type(self):
        return 'signal_rangemap_env_{}'.format(self._proc_id)

    def _get_title(self):
        return 'Mapping envelope'

    def _get_enabled(self):
        return self._get_range_map_params().get_envelope_enabled()

    def _set_enabled(self, enabled):
        self._get_range_map_params().set_envelope_enabled(enabled)

    def _get_envelope_data(self):
        return self._get_range_map_params().get_envelope()

    def _set_envelope_data(self, envelope):
        self._get_range_map_params().set_envelope(envelope)


