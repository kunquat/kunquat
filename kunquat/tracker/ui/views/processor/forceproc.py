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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from procnumslider import ProcNumSlider


class ForceProc(QWidget):

    @staticmethod
    def get_name():
        return u'Force'

    def __init__(self):
        QWidget.__init__(self)

        self._global_force = GlobalForceSlider()
        self._force_variation = ForceVarSlider()

        sliders = QGridLayout()
        sliders.addWidget(QLabel('Global force:'), 0, 0)
        sliders.addWidget(self._global_force, 0, 1)
        sliders.addWidget(QLabel('Force variation:'), 1, 0)
        sliders.addWidget(self._force_variation, 1, 1)

        v = QVBoxLayout()
        v.addLayout(sliders)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._global_force.set_au_id(au_id)
        self._force_variation.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._global_force.set_proc_id(proc_id)
        self._force_variation.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._global_force.set_ui_model(ui_model)
        self._force_variation.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._force_variation.unregister_updaters()
        self._global_force.unregister_updaters()


class ForceNumSlider(ProcNumSlider):

    def __init__(self, decimals, min_value, max_value):
        ProcNumSlider.__init__(self, decimals, min_value, max_value, '')

    def _get_force_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        force_params = proc.get_type_params()
        return force_params


class GlobalForceSlider(ForceNumSlider):

    def __init__(self):
        ForceNumSlider.__init__(self, 1, -64.0, 18.0)

    def _get_update_signal_type(self):
        return '_'.join(('signal_force_global_force', self._proc_id))

    def _update_value(self):
        force_params = self._get_force_params()
        self.set_number(force_params.get_global_force())

    def _value_changed(self, value):
        force_params = self._get_force_params()
        force_params.set_global_force(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class ForceVarSlider(ForceNumSlider):

    def __init__(self):
        ForceNumSlider.__init__(self, 1, 0.0, 32.0)

    def _get_update_signal_type(self):
        return '_'.join(('signal_force_force_variation', self._proc_id))

    def _update_value(self):
        force_params = self._get_force_params()
        self.set_number(force_params.get_force_variation())

    def _value_changed(self, value):
        force_params = self._get_force_params()
        force_params.set_force_variation(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))


