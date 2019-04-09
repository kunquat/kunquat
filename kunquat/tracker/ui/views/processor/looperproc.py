# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2018-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.varprecspinbox import VarPrecSpinBox
from .processorupdater import ProcessorUpdater
from .procnumslider import ProcNumSlider
from . import utils


class LooperProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Live looping'

    def __init__(self):
        super().__init__()

        self._max_rec_time = MaxRecTime()
        self._state_xfade_time = StateXFadeSlider()
        self._play_xfade_time = PlayXFadeSlider()

        self.add_to_updaters(
                self._max_rec_time, self._state_xfade_time, self._play_xfade_time)

        self._sliders_layout = QGridLayout()
        self._sliders_layout.setContentsMargins(0, 0, 0, 0)
        self._sliders_layout.setVerticalSpacing(0)
        self._sliders_layout.setColumnStretch(0, 0)
        self._sliders_layout.setColumnStretch(1, 1)
        self._sliders_layout.addWidget(QLabel('Maximum recording time:'), 0, 0)
        self._sliders_layout.addWidget(self._max_rec_time, 0, 1)
        self._sliders_layout.addWidget(QLabel('State crossfade time:'), 1, 0)
        self._sliders_layout.addWidget(self._state_xfade_time, 1, 1)
        self._sliders_layout.addWidget(QLabel('Playback crossfade time:'), 2, 0)
        self._sliders_layout.addWidget(self._play_xfade_time, 2, 1)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addLayout(self._sliders_layout)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        self._sliders_layout.setHorizontalSpacing(
                style_mgr.get_scaled_size_param('large_padding'))

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))


class MaxRecTime(VarPrecSpinBox, ProcessorUpdater):

    def __init__(self):
        super().__init__(step_decimals=0, max_decimals=1)
        self.setMinimum(0.1)
        self.setMaximum(60)
        self.setValue(16)

    def _get_update_signal_type(self):
        return 'signal_proc_looper_{}'.format(self._proc_id)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_value)
        self.register_action('signal_style_changed', self._update_style)
        self._update_value()
        self.valueChanged.connect(self._value_changed)

    def _update_style(self):
        self.update_style(self._ui_model.get_style_manager())

    def _update_value(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        old_block = self.blockSignals(True)
        new_rec_time = params.get_max_rec_time()
        if new_rec_time != self.value():
            self.setValue(new_rec_time)
        self.blockSignals(old_block)

    def _value_changed(self, value):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        params.set_max_rec_time(value)
        self._updater.signal_update(self._get_update_signal_type())


class XFadeSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(4, 0, 30, title='')
        self.set_number(0.005)

    def _get_looper_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        looper_params = proc.get_type_params()
        return looper_params


class StateXFadeSlider(XFadeSlider):

    def __init__(self):
        super().__init__()

    def _get_update_signal_type(self):
        return 'signal_looper_state_xfade_{}'.format(self._proc_id)

    def _update_value(self):
        lparams = self._get_looper_params()
        self.set_number(lparams.get_state_xfade_time())

    def _value_changed(self, value):
        lparams = self._get_looper_params()
        lparams.set_state_xfade_time(value)
        self._updater.signal_update(self._get_update_signal_type())


class PlayXFadeSlider(XFadeSlider):

    def __init__(self):
        super().__init__()

    def _get_update_signal_type(self):
        return 'signal_looper_play_xfade_{}'.format(self._proc_id)

    def _update_value(self):
        lparams = self._get_looper_params()
        self.set_number(lparams.get_play_xfade_time())

    def _value_changed(self, value):
        lparams = self._get_looper_params()
        lparams.set_play_xfade_time(value)
        self._updater.signal_update(self._get_update_signal_type())


