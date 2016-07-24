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

from .procnumslider import ProcNumSlider
from . import utils


class KsProc(QWidget):

    @staticmethod
    def get_name():
        return 'Karplus-Strong'

    def __init__(self):
        super().__init__()
        self._damp = DampSlider()

        sliders = QGridLayout()
        sliders.addWidget(QLabel('Damp:'), 0, 0)
        sliders.addWidget(self._damp, 0, 1)

        v = QVBoxLayout()
        v.addLayout(sliders)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._damp.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._damp.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._damp.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._damp.unregister_updaters()


class DampSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(1, 0.0, 100.0, '')

    def _get_ks_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_ks_damp_{}'.format(self._proc_id)

    def _update_value(self):
        ks_params = self._get_ks_params()
        self.set_number(ks_params.get_damp())

    def _value_changed(self, value):
        ks_params = self._get_ks_params()
        ks_params.set_damp(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))


