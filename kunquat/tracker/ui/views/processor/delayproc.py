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

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.headerline import HeaderLine
from .procnumslider import ProcNumSlider
from .processorupdater import ProcessorUpdater
from . import utils


class DelayProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Delay'

    def __init__(self):
        super().__init__()

        self._max_delay = MaxDelay()
        self._init_delay = InitDelay()

        self.add_to_updaters(self._max_delay, self._init_delay)

        gl = QGridLayout()
        gl.addWidget(QLabel('Maximum delay:'), 0, 0)
        gl.addWidget(self._max_delay, 0, 1)
        gl.addWidget(QLabel('Initial delay:'), 1, 0)
        gl.addWidget(self._init_delay, 1, 1)

        v = QVBoxLayout()
        v.addLayout(gl)
        v.addStretch(1)
        self.setLayout(v)


class MaxDelay(QDoubleSpinBox, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self.setDecimals(4)
        self.setMinimum(0.001)
        self.setMaximum(60)
        self.setValue(1)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_value)
        self._update_value()
        self.valueChanged.connect(self._value_changed)

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_delay', self._proc_id))

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
        self._updater.signal_update(self._get_update_signal_type())


class InitDelay(ProcNumSlider):

    def __init__(self):
        super().__init__(4, 0, 60, title='')
        self.set_number(0)

    def _update_value(self):
        delay_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        self.set_range(0, delay_params.get_max_delay())
        self.set_number(delay_params.get_init_delay())

    def _value_changed(self, value):
        delay_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        delay_params.set_init_delay(value)
        self._updater.signal_update(self._get_update_signal_type())

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_delay', self._proc_id))


