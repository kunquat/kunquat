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

from kunquat.tracker.ui.qt import *

from .processorupdater import ProcessorUpdater


class StreamProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Stream'

    def __init__(self):
        super().__init__()

        self._init_state_editor = InitStateEditor()

        self.add_to_updaters(self._init_state_editor)

        v = QVBoxLayout()
        v.addWidget(self._init_state_editor)
        v.addStretch(1)
        self.setLayout(v)


class InitStateEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._init_val = QDoubleSpinBox()
        self._init_val.setMinimum(-99999)
        self._init_val.setMaximum(99999)
        self._init_val.setDecimals(5)

        self._osc_speed = QDoubleSpinBox()
        self._osc_speed.setMinimum(0)
        self._osc_speed.setMaximum(1000)
        self._osc_speed.setDecimals(5)

        self._osc_depth = QDoubleSpinBox()
        self._osc_depth.setMinimum(0)
        self._osc_depth.setMaximum(99999)
        self._osc_depth.setDecimals(5)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(10)
        h.addWidget(QLabel('Initial value:'), 0)
        h.addWidget(self._init_val, 1)
        h.addWidget(QLabel('Initial oscillation speed:'), 0)
        h.addWidget(self._osc_speed, 1)
        h.addWidget(QLabel('Initial oscillation depth:'), 0)
        h.addWidget(self._osc_depth, 1)

        self.setLayout(h)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_state)

        self._init_val.valueChanged.connect(self._set_init_value)
        self._osc_speed.valueChanged.connect(self._set_osc_speed)
        self._osc_depth.valueChanged.connect(self._set_osc_depth)

        self._update_state()

    def _get_update_signal_type(self):
        return 'signal_stream_init_state_{}'.format(self._proc_id)

    def _get_stream_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        stream_params = proc.get_type_params()
        return stream_params

    def _update_state(self):
        stream_params = self._get_stream_params()

        init_value = stream_params.get_init_value()
        if init_value != self._init_val.value():
            old_block = self._init_val.blockSignals(True)
            self._init_val.setValue(init_value)
            self._init_val.blockSignals(old_block)

        osc_speed = stream_params.get_init_osc_speed()
        if osc_speed != self._osc_speed.value():
            old_block = self._osc_speed.blockSignals(True)
            self._osc_speed.setValue(osc_speed)
            self._osc_speed.blockSignals(old_block)

        osc_depth = stream_params.get_init_osc_depth()
        if osc_depth != self._osc_depth.value():
            old_block = self._osc_depth.blockSignals(True)
            self._osc_depth.setValue(osc_depth)
            self._osc_depth.blockSignals(old_block)

    def _set_init_value(self, value):
        stream_params = self._get_stream_params()
        stream_params.set_init_value(value)
        self._updater.signal_update(self._get_update_signal_type())

    def _set_osc_speed(self, value):
        stream_params = self._get_stream_params()
        stream_params.set_init_osc_speed(value)
        self._updater.signal_update(self._get_update_signal_type())

    def _set_osc_depth(self, value):
        stream_params = self._get_stream_params()
        stream_params.set_init_osc_depth(value)
        self._updater.signal_update(self._get_update_signal_type())


