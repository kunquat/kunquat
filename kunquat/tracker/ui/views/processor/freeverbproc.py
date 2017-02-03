# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
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

from .procnumslider import ProcNumSlider
from .processorupdater import ProcessorUpdater


class FreeverbProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Freeverb'

    def __init__(self):
        super().__init__()
        self._refl = ReflSlider()
        self._damp = DampSlider()

        self.add_updating_child(self._refl, self._damp)

        sliders = QGridLayout()
        sliders.addWidget(QLabel('Reflectivity'), 0, 0)
        sliders.addWidget(self._refl, 0, 1)
        sliders.addWidget(QLabel('Damp'), 1, 0)
        sliders.addWidget(self._damp, 1, 1)

        v = QVBoxLayout()
        v.addLayout(sliders)
        v.addStretch(1)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)


class FreeverbSlider(ProcNumSlider):

    def __init__(self, decimals, min_value, max_value):
        super().__init__(decimals, min_value, max_value, '')

    def _get_fv_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        freeverb_params = proc.get_type_params()
        return freeverb_params


class ReflSlider(FreeverbSlider):

    def __init__(self):
        super().__init__(1, 0, 200)

    def _get_update_signal_type(self):
        return '_'.join(('signal_freeverb_refl', self._proc_id))

    def _update_value(self):
        fv_params = self._get_fv_params()
        self.set_number(fv_params.get_reflectivity())

    def _value_changed(self, value):
        fv_params = self._get_fv_params()
        fv_params.set_reflectivity(value)
        self._updater.signal_update(self._get_update_signal_type())


class DampSlider(FreeverbSlider):

    def __init__(self):
        super().__init__(1, 0, 100)

    def _get_update_signal_type(self):
        return '_'.join(('signal_freeverb_damp', self._proc_id))

    def _update_value(self):
        fv_params = self._get_fv_params()
        self.set_number(fv_params.get_damp())

    def _value_changed(self, value):
        fv_params = self._get_fv_params()
        fv_params.set_damp(value)
        self._updater.signal_update(self._get_update_signal_type())


