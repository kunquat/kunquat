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

from PySide.QtCore import *
from PySide.QtGui import *

from . import utils
from .processorupdater import ProcessorUpdater
from .procnumslider import ProcNumSlider


class SlopeProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Slope'

    def __init__(self):
        super().__init__()
        self._smoothing = SmoothingSlider()

        self.add_to_updaters(self._smoothing)

        v = QVBoxLayout()
        v.addWidget(self._smoothing)
        v.addStretch(1)
        self.setLayout(v)


class SmoothingSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(2, 0.0, 10.0, 'Smoothing:')

    def _get_update_signal_type(self):
        return 'signal_slope_smoothing_{}'.format(self._proc_id)

    def _get_slope_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        slope_params = proc.get_type_params()
        return slope_params

    def _update_value(self):
        slope_params = self._get_slope_params()
        self.set_number(slope_params.get_smoothing())

    def _value_changed(self, value):
        slope_params = self._get_slope_params()
        slope_params.set_smoothing(value)
        self._updater.signal_update(self._get_update_signal_type())


