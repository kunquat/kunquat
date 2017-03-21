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

from kunquat.tracker.ui.views.envelope import Envelope
from kunquat.tracker.ui.views.headerline import HeaderLine
from .procnumslider import ProcNumSlider
from .proctimeenv import ProcessorTimeEnvelope
from .processorupdater import ProcessorUpdater
from . import utils


class EnvgenProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Envelope generation'

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._global_adjust = GlobalAdjustSlider()
        self._linear_force = QCheckBox('Linear force')
        self._range = RangeEditor()
        self._triggers = Triggers()
        self._time_env = EgenTimeEnv()

        rl = QHBoxLayout()
        rl.addWidget(self._linear_force)
        rl.addWidget(self._range)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._global_adjust)
        v.addLayout(rl)
        v.addWidget(self._triggers)
        v.addWidget(self._time_env)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def _on_setup(self):
        self.add_to_updaters(
                self._global_adjust, self._range, self._triggers, self._time_env)
        self.register_action(self._get_update_signal_type(), self._update_linear_force)

        QObject.connect(
                self._linear_force,
                SIGNAL('stateChanged(int)'),
                self._change_linear_force)

        self._update_linear_force()

    def _get_update_signal_type(self):
        return 'signal_egen_linear_force_{}'.format(self._proc_id)

    def _update_linear_force(self):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        enabled = egen_params.get_linear_force_enabled()

        self._range.setEnabled(not enabled)

        old_block = self._linear_force.blockSignals(True)
        self._linear_force.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self._linear_force.blockSignals(old_block)

    def _change_linear_force(self, state):
        enabled = (state == Qt.Checked)

        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        egen_params.set_linear_force_enabled(enabled)
        self._updater.signal_update(self._get_update_signal_type())


# TODO: change this into a spinbox -- a proper range is too wide for a slider
class GlobalAdjustSlider(ProcNumSlider):

    def __init__(self):
        super().__init__(2, -128.0, 128.0, title='Global adjust')
        self.set_number(0)

    def _update_value(self):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        self.set_number(egen_params.get_global_adjust())

    def _value_changed(self, value):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        egen_params.set_global_adjust(value)
        self._updater.signal_update(self._get_update_signal_type())

    def _get_update_signal_type(self):
        return 'signal_egen_global_adjust_{}'.format(self._proc_id)


class RangeEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._min_editor = QDoubleSpinBox()
        self._max_editor = QDoubleSpinBox()

        for editor in (self._min_editor, self._max_editor):
            editor.setMinimum(-99999)
            editor.setMaximum(99999)

        h = QHBoxLayout()
        h.setContentsMargins(10, 0, 10, 0)
        h.addWidget(QLabel('Minimum value:'), 0)
        h.addWidget(self._min_editor, 1)
        h.addWidget(QLabel('Maximum value:'), 0)
        h.addWidget(self._max_editor, 1)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_range)

        QObject.connect(
                self._min_editor, SIGNAL('valueChanged(double)'), self._set_range_min)
        QObject.connect(
                self._max_editor, SIGNAL('valueChanged(double)'), self._set_range_max)

        self._update_range()

    def _get_update_signal_type(self):
        return '_'.join(('signal_env_range', self._proc_id))

    def _update_range(self):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        y_range = egen_params.get_y_range()

        if y_range[0] != self._min_editor.value():
            old_block = self._min_editor.blockSignals(True)
            self._min_editor.setValue(y_range[0])
            self._min_editor.blockSignals(old_block)

        if y_range[1] != self._max_editor.value():
            old_block = self._max_editor.blockSignals(True)
            self._max_editor.setValue(y_range[1])
            self._max_editor.blockSignals(old_block)

    def _set_range_min(self, value):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        y_range = egen_params.get_y_range()
        y_range[0] = value
        y_range[1] = max(y_range)
        egen_params.set_y_range(y_range)
        self._updater.signal_update(self._get_update_signal_type())

    def _set_range_max(self, value):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        y_range = egen_params.get_y_range()
        y_range[1] = value
        y_range[0] = min(y_range)
        egen_params.set_y_range(y_range)
        self._updater.signal_update(self._get_update_signal_type())


class Triggers(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._immediate = QCheckBox('Immediately')
        self._release = QCheckBox('On release')
        self._impulse_floor = QCheckBox('On signal floor,')
        self._impulse_floor_bounds = TriggerImpulseFloorBounds()
        self._impulse_ceil = QCheckBox('On signal ceiling,')
        self._impulse_ceil_bounds = TriggerImpulseCeilBounds()

        gl = QGridLayout()
        gl.setContentsMargins(12, 0, 0, 0)
        gl.setVerticalSpacing(2)
        gl.setHorizontalSpacing(8)
        gl.setColumnStretch(2, 1)
        gl.addWidget(self._immediate, 0, 0)
        gl.addWidget(QWidget(), 0, 2)
        gl.addWidget(self._release, 1, 0)
        gl.addWidget(self._impulse_floor, 2, 0)
        gl.addWidget(self._impulse_floor_bounds, 2, 1)
        gl.addWidget(self._impulse_ceil, 3, 0)
        gl.addWidget(self._impulse_ceil_bounds, 3, 1)

        v = QVBoxLayout()
        v.setContentsMargins(2, 2, 2, 2)
        v.setSpacing(4)
        v.addWidget(QLabel('Trigger:'))
        v.addLayout(gl)
        self.setLayout(v)

    def _get_update_signal_type(self):
        return 'signal_env_trig_{}'.format(self._proc_id)

    def _on_setup(self):
        self.add_to_updaters(self._impulse_floor_bounds, self._impulse_ceil_bounds)
        self.register_action(self._get_update_signal_type(), self._update_all)

        QObject.connect(
                self._immediate, SIGNAL('stateChanged(int)'), self._change_immediate)
        QObject.connect(
                self._release, SIGNAL('stateChanged(int)'), self._change_release)
        QObject.connect(
                self._impulse_floor,
                SIGNAL('stateChanged(int)'),
                self._change_impulse_floor)
        QObject.connect(
                self._impulse_ceil,
                SIGNAL('stateChanged(int)'),
                self._change_impulse_ceil)

        self._update_all()

    def _get_egen_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_all(self):
        egen_params = self._get_egen_params()

        old_block = self._immediate.blockSignals(True)
        immediate = egen_params.get_trig_immediate()
        self._immediate.setCheckState(Qt.Checked if immediate else Qt.Unchecked)
        self._immediate.blockSignals(old_block)

        old_block = self._release.blockSignals(True)
        release = egen_params.get_trig_release()
        self._release.setCheckState(Qt.Checked if release else Qt.Unchecked)
        self._release.blockSignals(old_block)

        old_block = self._impulse_floor.blockSignals(True)
        ifloor = egen_params.get_trig_impulse_floor()
        self._impulse_floor.setCheckState(Qt.Checked if ifloor else Qt.Unchecked)
        self._impulse_floor.blockSignals(old_block)

        old_block = self._impulse_ceil.blockSignals(True)
        iceil = egen_params.get_trig_impulse_ceil()
        self._impulse_ceil.setCheckState(Qt.Checked if iceil else Qt.Unchecked)
        self._impulse_ceil.blockSignals(old_block)

    def _change_immediate(self, state):
        enabled = (state == Qt.Checked)

        self._get_egen_params().set_trig_immediate(enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _change_release(self, state):
        enabled = (state == Qt.Checked)

        self._get_egen_params().set_trig_release(enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _change_impulse_floor(self, state):
        enabled = (state == Qt.Checked)

        self._get_egen_params().set_trig_impulse_floor(enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _change_impulse_ceil(self, state):
        enabled = (state == Qt.Checked)

        self._get_egen_params().set_trig_impulse_ceil(enabled)
        self._updater.signal_update(self._get_update_signal_type())


class TriggerImpulseBounds(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._start_value = QDoubleSpinBox()
        self._stop_value = QDoubleSpinBox()

        self._start_value.setRange(-99999, 99999)
        self._stop_value.setRange(-99999, 99999)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(8)
        h.addWidget(QLabel('Start value:'))
        h.addWidget(self._start_value, 1)
        h.addWidget(QLabel('Stop value:'))
        h.addWidget(self._stop_value, 1)
        self.setLayout(h)

    def _get_update_signal_type(self):
        return 'signal_env_trig_{}'.format(self._proc_id)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_all)

        QObject.connect(
                self._start_value, SIGNAL('valueChanged(double)'), self._change_start)
        QObject.connect(
                self._stop_value, SIGNAL('valueChanged(double)'), self._change_stop)

        self._update_all()

    def _get_egen_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_all(self):
        self._update_enabled()
        self._update_bounds()

    # Protected callbacks

    def _update_enabled(self):
        raise NotImplementedError

    def _update_bounds(self):
        raise NotImplementedError

    def _change_start(self, new_start):
        raise NotImplementedError

    def _change_stop(self, new_stop):
        raise NotImplementedError


class TriggerImpulseFloorBounds(TriggerImpulseBounds):

    def __init__(self):
        super().__init__()

    def _update_enabled(self):
        egen_params = self._get_egen_params()
        self.setEnabled(egen_params.get_trig_impulse_floor())

    def _update_bounds(self):
        egen_params = self._get_egen_params()
        bounds = egen_params.get_trig_impulse_floor_bounds()

        old_block = self._start_value.blockSignals(True)
        self._start_value.setValue(bounds[0])
        self._start_value.blockSignals(old_block)

        old_block = self._stop_value.blockSignals(True)
        self._stop_value.setValue(bounds[1])
        self._stop_value.blockSignals(old_block)

    def _change_start(self, new_start):
        egen_params = self._get_egen_params()
        cur_bounds = egen_params.get_trig_impulse_floor_bounds()

        new_stop = max(cur_bounds[1], new_start)
        egen_params.set_trig_impulse_floor_bounds([new_start, new_stop])
        self._updater.signal_update(self._get_update_signal_type())

    def _change_stop(self, new_stop):
        egen_params = self._get_egen_params()
        cur_bounds = egen_params.get_trig_impulse_floor_bounds()

        new_start = min(cur_bounds[0], new_stop)
        egen_params.set_trig_impulse_floor_bounds([new_start, new_stop])
        self._updater.signal_update(self._get_update_signal_type())


class TriggerImpulseCeilBounds(TriggerImpulseBounds):

    def __init__(self):
        super().__init__()

    def _update_enabled(self):
        egen_params = self._get_egen_params()
        self.setEnabled(egen_params.get_trig_impulse_ceil())

    def _update_bounds(self):
        egen_params = self._get_egen_params()
        bounds = egen_params.get_trig_impulse_ceil_bounds()

        old_block = self._start_value.blockSignals(True)
        self._start_value.setValue(bounds[0])
        self._start_value.blockSignals(old_block)

        old_block = self._stop_value.blockSignals(True)
        self._stop_value.setValue(bounds[1])
        self._stop_value.blockSignals(old_block)

    def _change_start(self, new_start):
        egen_params = self._get_egen_params()
        cur_bounds = egen_params.get_trig_impulse_ceil_bounds()

        new_stop = min(cur_bounds[1], new_start)
        egen_params.set_trig_impulse_ceil_bounds([new_start, new_stop])
        self._updater.signal_update(self._get_update_signal_type())

    def _change_stop(self, new_stop):
        egen_params = self._get_egen_params()
        cur_bounds = egen_params.get_trig_impulse_ceil_bounds()

        new_start = max(cur_bounds[0], new_stop)
        egen_params.set_trig_impulse_ceil_bounds([new_start, new_stop])
        self._updater.signal_update(self._get_update_signal_type())


class EgenTimeEnv(ProcessorTimeEnvelope):

    def __init__(self):
        super().__init__()
        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def _get_title(self):
        return 'Envelope'

    def _allow_loop(self):
        return True

    def _make_envelope_widget(self):
        envelope = Envelope()
        envelope.set_node_count_max(32)
        envelope.set_y_range(0, 1)
        envelope.set_x_range(0, 4)
        envelope.set_first_lock(True, False)
        envelope.set_x_range_adjust(False, True)
        return envelope

    def _get_update_signal_type(self):
        return ''.join(('signal_egen_time_env_', self._au_id, self._proc_id))

    def _get_egen_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_enabled(self):
        return self._get_egen_params().get_time_env_enabled()

    def _set_enabled(self, enabled):
        self._get_egen_params().set_time_env_enabled(enabled)

    def _get_loop_enabled(self):
        return self._get_egen_params().get_time_env_loop_enabled()

    def _set_loop_enabled(self, enabled):
        self._get_egen_params().set_time_env_loop_enabled(enabled)

    def _get_envelope_data(self):
        return self._get_egen_params().get_time_env()

    def _set_envelope_data(self, envelope):
        self._get_egen_params().set_time_env(envelope)


