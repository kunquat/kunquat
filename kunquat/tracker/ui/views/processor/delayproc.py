# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
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

from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.headerline import HeaderLine
from .procnumslider import ProcNumSlider
from . import utils


class DelayProc(QWidget):

    @staticmethod
    def get_name():
        return 'Delay'

    def __init__(self):
        QWidget.__init__(self)

        self._max_delay = MaxDelay()
        self._init_delay = InitDelay()

        gl = QGridLayout()
        gl.addWidget(QLabel('Maximum delay:'), 0, 0)
        gl.addWidget(self._max_delay, 0, 1)
        gl.addWidget(QLabel('Initial delay:'), 1, 0)
        gl.addWidget(self._init_delay, 1, 1)

        v = QVBoxLayout()
        v.addLayout(gl)
        v.addStretch(1)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._max_delay.set_au_id(au_id)
        self._init_delay.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._max_delay.set_proc_id(proc_id)
        self._init_delay.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._max_delay.set_ui_model(ui_model)
        self._init_delay.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._init_delay.unregister_updaters()
        self._max_delay.unregister_updaters()


class MaxDelay(QDoubleSpinBox):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self.setDecimals(4)
        self.setMinimum(0.001)
        self.setMaximum(60)
        self.setValue(1)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_value()
        QObject.connect(self, SIGNAL('valueChanged(double)'), self._value_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_delay', self._proc_id))

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_value()

    def _update_value(self):
        delay_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        old_block = self.blockSignals(True)
        new_delay = delay_params.get_max_delay()
        if new_delay != self.value():
            self.setValue(new_delay)
        self.blockSignals(old_block)

    def _value_changed(self, value):
        delay_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        delay_params.set_max_delay(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class InitDelay(ProcNumSlider):

    def __init__(self):
        ProcNumSlider.__init__(self, 4, 0, 60, title='')
        self.set_number(0)

    def _update_value(self):
        delay_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        self.set_range(0, delay_params.get_max_delay())
        self.set_number(delay_params.get_init_delay())

    def _value_changed(self, value):
        delay_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        delay_params.set_init_delay(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_delay', self._proc_id))


