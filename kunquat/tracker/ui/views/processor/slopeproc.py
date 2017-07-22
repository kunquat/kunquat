# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from . import utils
from .processorupdater import ProcessorUpdater
from .procnumslider import ProcNumSlider


class SlopeProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Slope'

    def __init__(self):
        super().__init__()
        self._absolute = AbsoluteToggle()
        self._smoothing = SmoothingSlider()

        self.add_to_updaters(self._absolute, self._smoothing)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(8)
        v.addWidget(self._absolute)
        v.addWidget(self._smoothing)
        v.addStretch(1)
        self.setLayout(v)


class AbsoluteToggle(QCheckBox, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self.setText('Absolute')

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_absolute)

        self.stateChanged.connect(self._change_absolute)

        self._update_absolute()

    def _get_update_signal_type(self):
        return 'signal_slope_absolute_{}'.format(self._proc_id)

    def _get_slope_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_absolute(self):
        slope_params = self._get_slope_params()

        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if slope_params.get_absolute() else Qt.Unchecked)
        self.blockSignals(old_block)

    def _change_absolute(self, state):
        enabled = (state == Qt.Checked)
        slope_params = self._get_slope_params()
        slope_params.set_absolute(enabled)
        self._updater.signal_update(self._get_update_signal_type())


class SmoothingSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(2, 0.0, 10.0, 'Smoothing:')

    def _get_update_signal_type(self):
        return 'signal_slope_smoothing_{}'.format(self._proc_id)

    def _get_slope_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_value(self):
        slope_params = self._get_slope_params()
        self.set_number(slope_params.get_smoothing())

    def _value_changed(self, value):
        slope_params = self._get_slope_params()
        slope_params.set_smoothing(value)
        self._updater.signal_update(self._get_update_signal_type())


