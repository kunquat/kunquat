# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2019
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
from kunquat.tracker.ui.views.varprecspinbox import VarPrecSpinBox
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

        self._controls_layout = QGridLayout()
        self._controls_layout.setContentsMargins(0, 0, 0, 0)
        self._controls_layout.addWidget(QLabel('Maximum delay:'), 0, 0)
        self._controls_layout.addWidget(self._max_delay, 0, 1)
        self._controls_layout.addWidget(QLabel('Initial delay:'), 1, 0)
        self._controls_layout.addWidget(self._init_delay, 1, 1)

        v = QVBoxLayout()
        v.addLayout(self._controls_layout)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        self._controls_layout.setHorizontalSpacing(
                style_mgr.get_scaled_size_param('medium_padding'))
        self._controls_layout.setVerticalSpacing(
                style_mgr.get_scaled_size_param('small_padding'))

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)


class MaxDelay(VarPrecSpinBox, ProcessorUpdater):

    def __init__(self):
        super().__init__(step_decimals=1, max_decimals=3)
        self.setMinimum(0.001)
        self.setMaximum(60)
        self.setValue(1)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_value)
        self.register_action('signal_style_changed', self._update_style)
        self._update_value()
        self.valueChanged.connect(self._value_changed)

    def _update_style(self):
        self.update_style(self._ui_model.get_style_manager())

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


