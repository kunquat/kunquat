# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
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


class PanningProc(QWidget):

    @staticmethod
    def get_name():
        return 'Panning'

    def __init__(self):
        super().__init__()

        self._panning = PanningSlider()

        v = QVBoxLayout()
        v.addWidget(self._panning)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._panning.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._panning.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._panning.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._panning.unregister_updaters();


class PanningSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(2, -1.0, 1.0, title='Panning')

    def _get_panning_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        panning_params = proc.get_type_params()
        return panning_params

    def _get_update_signal_type(self):
        return '_'.join(('signal_panning_panning', self._proc_id))

    def _update_value(self):
        panning_params = self._get_panning_params()
        self.set_number(panning_params.get_panning())

    def _value_changed(self, panning):
        panning_params = self._get_panning_params()
        panning_params.set_panning(panning)
        self._updater.signal_update(self._get_update_signal_type())


