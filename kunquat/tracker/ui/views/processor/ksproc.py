# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from .procnumslider import ProcNumSlider
from .processorupdater import ProcessorUpdater
from . import utils


class KsProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Karplus-Strong'

    def __init__(self):
        super().__init__()
        self._damp = DampSlider()

        self._arr_toggle = AudioRateRangeToggle()
        self._arr = AudioRateRange()

        self.add_to_updaters(self._damp, self._arr_toggle, self._arr)

        self._sliders_layout = QGridLayout()
        self._sliders_layout.setContentsMargins(0, 0, 0, 0)
        self._sliders_layout.setVerticalSpacing(0)
        self._sliders_layout.addWidget(QLabel('Damp:'), 0, 0)
        self._sliders_layout.addWidget(self._damp, 0, 1)

        self._arr_layout = QHBoxLayout()
        self._arr_layout.setContentsMargins(0, 0, 0, 0)
        self._arr_layout.setSpacing(0)
        self._arr_layout.addWidget(self._arr_toggle)
        self._arr_layout.addWidget(self._arr)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addLayout(self._sliders_layout)
        v.addLayout(self._arr_layout)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        self._sliders_layout.setHorizontalSpacing(
                style_mgr.get_scaled_size_param('large_padding'))

        self._arr_layout.setSpacing(style_mgr.get_scaled_size_param('large_padding'))

        margin = style_mgr.get_scaled_size_param('medium_padding')
        self.layout().setContentsMargins(margin, margin, margin, margin)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))


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
        self._updater.signal_update(self._get_update_signal_type())


class AudioRateRangeToggle(QCheckBox, ProcessorUpdater):

    def __init__(self):
        super().__init__('Enable audio rate range')

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_enabled)

        self.stateChanged.connect(self._change_enabled)

        self._update_enabled()

    def _get_ks_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _get_update_signal_type(self):
        return 'signal_ks_audio_rate_range_{}'.format(self._proc_id)

    def _update_enabled(self):
        ks_params = self._get_ks_params()

        old_block = self.blockSignals(True)
        enabled = ks_params.get_audio_rate_range_enabled()
        self.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self.blockSignals(old_block)

    def _change_enabled(self, state):
        enabled = (state == Qt.Checked)
        ks_params = self._get_ks_params()
        ks_params.set_audio_rate_range_enabled(enabled)
        self._updater.signal_update(self._get_update_signal_type())


class AudioRateRange(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._minimum = AudioRateRangeValue()
        self._maximum = AudioRateRangeValue()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(0)
        h.addWidget(QLabel('Minimum:'))
        h.addWidget(self._minimum)
        h.addWidget(QLabel('Maximum:'))
        h.addWidget(self._maximum)
        h.addStretch(1)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self.register_action(self._get_update_signal_type(), self._update_all)

        self._minimum.editingFinished.connect(
                self._set_minimum, type=Qt.QueuedConnection)
        self._maximum.editingFinished.connect(
                self._set_maximum, type=Qt.QueuedConnection)

        self._update_style()
        self._update_all()

    def _get_update_signal_type(self):
        return 'signal_ks_audio_rate_range_{}'.format(self._proc_id)

    def _get_ks_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_all(self):
        ks_params = self._get_ks_params()

        self.setEnabled(ks_params.get_audio_rate_range_enabled())

        self._minimum.set_value(ks_params.get_audio_rate_range_min())
        self._maximum.set_value(ks_params.get_audio_rate_range_max())

    def _set_minimum(self):
        self._get_ks_params().set_audio_rate_range_min(self._minimum.value())
        self._updater.signal_update(self._get_update_signal_type())

    def _set_maximum(self):
        self._get_ks_params().set_audio_rate_range_max(self._maximum.value())
        self._updater.signal_update(self._get_update_signal_type())

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('large_padding'))


class AudioRateRangeValue(QSpinBox):

    _MIN_AUDIO_RATE = 1000
    _MAX_AUDIO_RATE = 384000

    def __init__(self):
        super().__init__()
        self.setRange(self._MIN_AUDIO_RATE, self._MAX_AUDIO_RATE)

    def set_value(self, value):
        if self.value() != value:
            old_block = self.blockSignals(True)
            self.setValue(value)
            self.blockSignals(old_block)


