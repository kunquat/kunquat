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
from .procsimpleenv import ProcessorSimpleEnvelope
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
        self._time_env = EgenTimeEnv()
        self._force_env = ForceEnv()

        rl = QHBoxLayout()
        rl.addWidget(self._linear_force)
        rl.addWidget(self._range)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._global_adjust)
        v.addLayout(rl)
        v.addWidget(self._time_env)
        #v.addWidget(self._force_env)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def _on_setup(self):
        self.add_to_updaters(
                self._global_adjust, self._range, self._time_env, self._force_env)
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

        self._force_env.setEnabled(enabled)
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


class EgenTimeEnv(ProcessorTimeEnvelope):

    def __init__(self):
        super().__init__()
        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def _get_title(self):
        return 'Envelope'

    def _allow_loop(self):
        return True

    def _allow_release_toggle(self):
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

    def _get_release_enabled(self):
        return self._get_egen_params().get_time_env_is_release()

    def _set_release_enabled(self, enabled):
        self._get_egen_params().set_time_env_is_release(enabled)

    def _get_scale_amount(self):
        return self._get_egen_params().get_time_env_scale_amount()

    def _set_scale_amount(self, value):
        self._get_egen_params().set_time_env_scale_amount(value)

    def _get_scale_centre(self):
        return self._get_egen_params().get_time_env_scale_centre()

    def _set_scale_centre(self, value):
        self._get_egen_params().set_time_env_scale_centre(value)

    def _get_envelope_data(self):
        return self._get_egen_params().get_time_env()

    def _set_envelope_data(self, envelope):
        self._get_egen_params().set_time_env(envelope)


class ForceEnv(ProcessorSimpleEnvelope):

    def __init__(self):
        super().__init__()

    def _get_update_signal_type(self):
        return ''.join(('signal_add_force_mod_volume_', self._au_id, self._proc_id))

    def _get_title(self):
        return 'Force envelope'

    def _make_envelope_widget(self):
        envelope = Envelope({ 'is_square_area': True })
        envelope.set_node_count_max(32)
        envelope.set_y_range(0, 1)
        envelope.set_x_range(0, 1)
        envelope.set_first_lock(True, False)
        envelope.set_last_lock(True, False)
        return envelope

    def _get_enabled(self):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        return egen_params.get_force_env_enabled()

    def _set_enabled(self, enabled):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        egen_params.set_force_env_enabled(enabled)

    def _get_envelope_data(self):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        return egen_params.get_force_env()

    def _set_envelope_data(self, envelope):
        egen_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        egen_params.set_force_env(envelope)


