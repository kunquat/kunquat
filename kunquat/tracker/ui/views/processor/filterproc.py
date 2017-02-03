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


class FilterProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Filter'

    def __init__(self):
        super().__init__()

        self._cutoff = CutoffSlider()
        self._resonance = ResonanceSlider()

        self.add_updating_child(self._cutoff, self._resonance)

        sliders = QGridLayout()
        sliders.addWidget(QLabel('Cutoff'), 0, 0)
        sliders.addWidget(self._cutoff, 0, 1)
        sliders.addWidget(QLabel('Resonance'), 1, 0)
        sliders.addWidget(self._resonance, 1, 1)

        v = QVBoxLayout()
        v.addLayout(sliders)
        v.addStretch(1)
        self.setLayout(v)


class FilterSlider(ProcNumSlider):

    def __init__(self, decimals, min_value, max_value):
        super().__init__(decimals, min_value, max_value, '')

    def _get_filter_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        filter_params = proc.get_type_params()
        return filter_params


class CutoffSlider(FilterSlider):

    def __init__(self):
        super().__init__(2, 0.0, 100.0)

    def _get_update_signal_type(self):
        return '_'.join(('signal_filter_cutoff', self._proc_id))

    def _update_value(self):
        filter_params = self._get_filter_params()
        self.set_number(filter_params.get_cutoff())

    def _value_changed(self, cutoff):
        filter_params = self._get_filter_params()
        filter_params.set_cutoff(cutoff)
        self._updater.signal_update(self._get_update_signal_type())


class ResonanceSlider(FilterSlider):

    def __init__(self):
        super().__init__(2, 0.0, 100.0)

    def _get_update_signal_type(self):
        return '_'.join(('signal_filter_resonance', self._proc_id))

    def _update_value(self):
        filter_params = self._get_filter_params()
        self.set_number(filter_params.get_resonance())

    def _value_changed(self, resonance):
        filter_params = self._get_filter_params()
        filter_params.set_resonance(resonance)
        self._updater.signal_update(self._get_update_signal_type())


