# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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

from . import utils


class RangeMapProc(QWidget):

    @staticmethod
    def get_name():
        return 'Range map'

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._from_min = RangeValue()
        self._from_max = RangeValue()
        self._min_to = RangeValue()
        self._max_to = RangeValue()
        self._clamp_dest_min = ClampToggle('Clamp to minimum destination')
        self._clamp_dest_max = ClampToggle('Clamp to maximum destination')

        gl = QGridLayout()
        gl.addWidget(QLabel('Source range minimum:'), 0, 0)
        gl.addWidget(self._from_min, 0, 1)
        gl.addWidget(QLabel('Source range maximum:'), 0, 2)
        gl.addWidget(self._from_max, 0, 3)
        gl.addWidget(QWidget(), 0, 4)
        gl.addWidget(QLabel('Map minimum to:'), 1, 0)
        gl.addWidget(self._min_to, 1, 1)
        gl.addWidget(QLabel('Map maximum to:'), 1, 2)
        gl.addWidget(self._max_to, 1, 3)

        v = QVBoxLayout()
        v.addLayout(gl)
        v.addWidget(self._clamp_dest_min)
        v.addWidget(self._clamp_dest_max)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._from_min, SIGNAL('valueChanged(double)'), self._set_from_min)
        QObject.connect(
                self._from_max, SIGNAL('valueChanged(double)'), self._set_from_max)
        QObject.connect(self._min_to, SIGNAL('valueChanged(double)'), self._set_min_to)
        QObject.connect(self._max_to, SIGNAL('valueChanged(double)'), self._set_max_to)

        QObject.connect(
                self._clamp_dest_min,
                SIGNAL('stateChanged(int)'),
                self._set_clamp_dest_min)
        QObject.connect(
                self._clamp_dest_max,
                SIGNAL('stateChanged(int)'),
                self._set_clamp_dest_max)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_rangemap_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_all()

    def _get_range_map_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_all(self):
        params = self._get_range_map_params()

        self._from_min.set_value(params.get_from_min())
        self._from_max.set_value(params.get_from_max())
        self._min_to.set_value(params.get_min_to())
        self._max_to.set_value(params.get_max_to())

        self._clamp_dest_min.set_enabled(params.get_clamp_dest_min())
        self._clamp_dest_max.set_enabled(params.get_clamp_dest_max())

    def _set_from_min(self, value):
        params = self._get_range_map_params()
        params.set_from_min(value)
        params.set_from_max(max(params.get_from_min(), params.get_from_max()))
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _set_from_max(self, value):
        params = self._get_range_map_params()
        params.set_from_max(value)
        params.set_from_min(min(params.get_from_min(), params.get_from_max()))
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _set_min_to(self, value):
        self._get_range_map_params().set_min_to(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _set_max_to(self, value):
        self._get_range_map_params().set_max_to(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _set_clamp_dest_min(self, state):
        enabled = (state == Qt.Checked)
        self._get_range_map_params().set_clamp_dest_min(enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _set_clamp_dest_max(self, state):
        enabled = (state == Qt.Checked)
        self._get_range_map_params().set_clamp_dest_max(enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class RangeValue(QDoubleSpinBox):

    def __init__(self):
        super().__init__()
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


