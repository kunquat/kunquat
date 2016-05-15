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

import math
import random

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.headerline import HeaderLine
from . import utils
from .procnumslider import ProcNumSlider
from .waveformeditor import WaveformEditor


class PadsynthProc(QWidget):

    @staticmethod
    def get_name():
        return 'PADsynth'

    def __init__(self):
        super().__init__()

        self._playback_params = PlaybackParams()
        self._apply_button = ApplyButton()
        self._sample_config = SampleConfigEditor()
        self._bandwidth = BandwidthEditor()
        self._harmonics_base = HarmonicsBaseEditor()
        self._harmonic_scales = HarmonicScales()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(4)
        v.addWidget(self._playback_params)
        v.addWidget(self._apply_button)
        v.addWidget(self._sample_config)
        v.addWidget(self._bandwidth)
        v.addWidget(self._harmonics_base)
        v.addWidget(self._harmonic_scales)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._playback_params.set_au_id(au_id)
        self._apply_button.set_au_id(au_id)
        self._sample_config.set_au_id(au_id)
        self._bandwidth.set_au_id(au_id)
        self._harmonics_base.set_au_id(au_id)
        self._harmonic_scales.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._playback_params.set_proc_id(proc_id)
        self._apply_button.set_proc_id(proc_id)
        self._sample_config.set_proc_id(proc_id)
        self._bandwidth.set_proc_id(proc_id)
        self._harmonics_base.set_proc_id(proc_id)
        self._harmonic_scales.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._playback_params.set_ui_model(ui_model)
        self._apply_button.set_ui_model(ui_model)
        self._sample_config.set_ui_model(ui_model)
        self._bandwidth.set_ui_model(ui_model)
        self._harmonics_base.set_ui_model(ui_model)
        self._harmonic_scales.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._harmonic_scales.unregister_updaters()
        self._harmonics_base.unregister_updaters()
        self._bandwidth.unregister_updaters()
        self._sample_config.unregister_updaters()
        self._apply_button.unregister_updaters()
        self._playback_params.unregister_updaters()


class PlaybackParams(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._ramp_attack = QCheckBox('Ramp attack')
        self._stereo = QCheckBox('Stereo')

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._ramp_attack)
        h.addWidget(self._stereo)
        h.addStretch(1)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._ramp_attack, SIGNAL('stateChanged(int)'), self._toggle_ramp_attack)
        QObject.connect(
                self._stereo, SIGNAL('stateChanged(int)'), self._toggle_stereo)

        self._update_all()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_padsynth_rt_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        old_block = self._ramp_attack.blockSignals(True)
        self._ramp_attack.setCheckState(
                Qt.Checked if params.get_ramp_attack_enabled() else Qt.Unchecked)
        self._ramp_attack.blockSignals(old_block)

        old_block = self._stereo.blockSignals(True)
        self._stereo.setCheckState(
                Qt.Checked if params.get_stereo_enabled() else Qt.Unchecked)
        self._stereo.blockSignals(old_block)

    def _toggle_ramp_attack(self, state):
        enabled = (state == Qt.Checked)
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        params.set_ramp_attack_enabled(enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _toggle_stereo(self, state):
        enabled = (state == Qt.Checked)
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        params.set_stereo_enabled(enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class ApplyButton(QPushButton):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self.setText('Apply parameters')
        self.setEnabled(False)

        self.setStyleSheet(
            '''QPushButton:enabled
            {
                background-color: #a32;
                color: #fff;
            }
            ''')

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('clicked()'), self._apply_params)

        self._update_status()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_status()

    def _update_status(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        self.setEnabled(not params.is_config_applied())

    def _apply_params(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        params.apply_config()
        self._updater.signal_update(set([self._get_update_signal_type()]))


class PadsynthParamSlider(ProcNumSlider):

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)


class SampleConfigEditor(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._sample_size = QComboBox()
        self._sample_count = QSpinBox()
        self._sample_count.setRange(1, 128)
        self._range_min = SamplePitchRangeMinEditor()
        self._range_max = SamplePitchRangeMaxEditor()
        self._center_pitch = SampleCenterPitchEditor()

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(QLabel('Sample size:'))
        h.addWidget(self._sample_size)
        h.addWidget(QLabel('Sample count:'))
        h.addWidget(self._sample_count)
        h.addWidget(QLabel('Pitch range:'))
        h.addWidget(self._range_min)
        h.addWidget(self._range_max)
        h.addWidget(QLabel('Center pitch:'))
        h.addWidget(self._center_pitch)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._range_min.set_au_id(au_id)
        self._range_max.set_au_id(au_id)
        self._center_pitch.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._range_min.set_proc_id(proc_id)
        self._range_max.set_proc_id(proc_id)
        self._center_pitch.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._range_min.set_ui_model(ui_model)
        self._range_max.set_ui_model(ui_model)
        self._center_pitch.set_ui_model(ui_model)

        for sample_length in self._get_params().get_allowed_sample_lengths():
            self._sample_size.addItem(str(sample_length), userData=sample_length)

        QObject.connect(
                self._sample_size,
                SIGNAL('currentIndexChanged(int)'),
                self._change_sample_size)

        QObject.connect(
                self._sample_count,
                SIGNAL('valueChanged(int)'),
                self._change_sample_count)

        self._update_sample_params()

    def unregister_updaters(self):
        self._center_pitch.unregister_updaters()
        self._range_max.unregister_updaters()
        self._range_min.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_sample_params()

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_sample_params(self):
        params = self._get_params()

        old_block = self._sample_size.blockSignals(True)
        new_sample_size = params.get_sample_length()
        if (self._sample_size.itemData(self._sample_size.currentIndex()) !=
                new_sample_size):
            self._sample_size.setCurrentIndex(
                    self._sample_size.findData(new_sample_size))
        self._sample_size.blockSignals(old_block)

        old_block = self._sample_count.blockSignals(True)
        new_sample_count = params.get_sample_count()
        if self._sample_count.value() != new_sample_count:
            self._sample_count.setValue(new_sample_count)
        self._sample_count.blockSignals(old_block)

    def _change_sample_size(self, index):
        sample_size = self._sample_size.itemData(index)
        self._get_params().set_sample_length(sample_size)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _change_sample_count(self, count):
        params = self._get_params()
        params.set_sample_count(count)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class SamplePitchRangeMinEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(0, -6000.0, 6000.0)

    def _update_value(self):
        min_pitch, _ = self._get_params().get_sample_pitch_range()
        self.set_number(min_pitch)

    def _value_changed(self, min_pitch):
        params = self._get_params()
        _, max_pitch = params.get_sample_pitch_range()
        max_pitch = max(min_pitch, max_pitch)
        params.set_sample_pitch_range(min_pitch, max_pitch)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class SamplePitchRangeMaxEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(0, -6000.0, 6000.0)

    def _update_value(self):
        _, max_pitch = self._get_params().get_sample_pitch_range()
        self.set_number(max_pitch)

    def _value_changed(self, max_pitch):
        params = self._get_params()
        min_pitch, _ = params.get_sample_pitch_range()
        min_pitch = min(min_pitch, max_pitch)
        params.set_sample_pitch_range(min_pitch, max_pitch)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class SampleCenterPitchEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(0, -6000.0, 6000.0)

    def _update_value(self):
        center_pitch = self._get_params().get_sample_center_pitch()
        self.set_number(center_pitch)

    def _value_changed(self, center_pitch):
        self._get_params().set_sample_center_pitch(center_pitch)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class BandwidthEditor(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._base = BandwidthBaseEditor()
        self._scale = BandwidthScaleEditor()

        g = QGridLayout()
        g.setContentsMargins(0, 0, 0, 0)
        g.setHorizontalSpacing(2)
        g.setVerticalSpacing(0)
        g.addWidget(QLabel('Bandwidth base:'), 0, 0)
        g.addWidget(self._base, 0, 1)
        g.addWidget(QLabel('Bandwidth scale:'), 1, 0)
        g.addWidget(self._scale, 1, 1)
        self.setLayout(g)

    def set_au_id(self, au_id):
        self._base.set_au_id(au_id)
        self._scale.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._base.set_proc_id(proc_id)
        self._scale.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._base.set_ui_model(ui_model)
        self._scale.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._scale.unregister_updaters()
        self._base.unregister_updaters()


class BandwidthBaseEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(1, 1.0, 1200.0)

    def _update_value(self):
        self.set_number(self._get_params().get_bandwidth_base())

    def _value_changed(self, bandwidth):
        self._get_params().set_bandwidth_base(bandwidth)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class BandwidthScaleEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(2, 1.0, 10.0)

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_value(self):
        self.set_number(self._get_params().get_bandwidth_scale())

    def _value_changed(self, scale):
        self._get_params().set_bandwidth_scale(scale)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class HarmonicsBaseEditor(WaveformEditor):

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_base_wave(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = params.get_harmonics_wave()
        return base_wave


class HarmonicScalesList(EditorList):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_all()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

    def _make_adder_widget(self):
        adder = HarmonicScaleAdder()
        adder.set_au_id(self._au_id)
        adder.set_proc_id(self._proc_id)
        adder.set_ui_model(self._ui_model)
        return adder

    def _get_updated_editor_count(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        count = params.get_harmonic_scales().get_count()
        return count

    def _make_editor_widget(self, index):
        editor = HarmonicScaleEditor(index)
        editor.set_au_id(self._au_id)
        editor.set_proc_id(self._proc_id)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        editor.update_index(index)

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        self.update_list()


class HarmonicScaleAdder(QPushButton):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self.setText('Add harmonic scale')

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('clicked()'), self._add_harmonic)

    def unregister_updaters(self):
        pass

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _add_harmonic(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        scales = params.get_harmonic_scales()
        scales.append_scale()
        self._updater.signal_update(set([self._get_update_signal_type()]))


class HarmonicScaleEditor(QWidget):

    def __init__(self, index):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._index = index

        self._pitch_factor = QDoubleSpinBox()
        self._pitch_factor.setDecimals(3)
        self._pitch_factor.setRange(0.001, 1024.0)
        self._pitch_factor.setValue(1)

        self._amplitude = AmplitudeEditor(index)

        self._remove_button = QPushButton()
        self._remove_button.setStyleSheet('padding: 0 -2px;')
        self._remove_button.setEnabled(self._index != 0)

        h = QHBoxLayout()
        h.setContentsMargins(4, 0, 4, 0)
        h.setSpacing(2)
        h.addWidget(QLabel('Pitch factor:'))
        h.addWidget(self._pitch_factor)
        h.addWidget(self._amplitude)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._amplitude.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._amplitude.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        self._amplitude.set_ui_model(ui_model)

        icon_bank = self._ui_model.get_icon_bank()
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))

        QObject.connect(
                self._pitch_factor,
                SIGNAL('valueChanged(double)'),
                self._change_pitch_factor)

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_harmonic)

        self.update_index(self._index)

    def unregister_updaters(self):
        self._amplitude.unregister_updaters()

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def update_index(self, index):
        self._index = index

        scales = self._get_params().get_harmonic_scales()
        if self._index >= scales.get_count():
            return

        scale = scales.get_scale(self._index)

        old_block = self._pitch_factor.blockSignals(True)
        new_pitch_factor = scale.get_freq_mul()
        if new_pitch_factor != self._pitch_factor.value():
            self._pitch_factor.setValue(new_pitch_factor)
        self._pitch_factor.blockSignals(old_block)

        self._remove_button.setEnabled(scales.get_count() > 1)

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _change_pitch_factor(self, value):
        scales = self._get_params().get_harmonic_scales()
        if self._index >= scales.get_count():
            return

        scale = scales.get_scale(self._index)
        scale.set_freq_mul(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _remove_harmonic(self):
        scales = self._get_params().get_harmonic_scales()
        if self._index >= scales.get_count():
            return

        scales.remove_scale(self._index)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class AmplitudeEditor(PadsynthParamSlider):

    def __init__(self, index):
        super().__init__(3, 0.0, 1.0, title='Amplitude:')
        self._index = index

    def _update_value(self):
        scales = self._get_params().get_harmonic_scales()
        if self._index >= scales.get_count():
            return

        scale = scales.get_scale(self._index)
        self.set_number(scale.get_amplitude())

    def _value_changed(self, amplitude):
        scales = self._get_params().get_harmonic_scales()
        if self._index >= scales.get_count():
            return

        scale = scales.get_scale(self._index)
        scale.set_amplitude(amplitude)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class HarmonicScales(QWidget):

    def __init__(self):
        super().__init__()

        self._editor = HarmonicScalesList()

        v = QVBoxLayout()
        v.addWidget(HeaderLine('Harmonic scales'))
        v.addWidget(self._editor)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._editor.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._editor.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._editor.unregister_updaters()

