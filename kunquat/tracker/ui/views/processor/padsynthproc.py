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

import math
import random

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.envelope import Envelope
from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.kqtcombobox import KqtComboBox
from kunquat.tracker.ui.views.stylecreator import StyleCreator
from kunquat.tracker.ui.views.varprecspinbox import VarPrecSpinBox
from . import utils
from .procnumslider import ProcNumSlider
from .processorupdater import ProcessorUpdater
from .prociconbutton import ProcessorIconButton
from .procsimpleenv import ProcessorSimpleEnvelope
from .waveformeditor import WaveformEditor


def _update_layout(style_mgr, layout):
    margin = style_mgr.get_scaled_size_param('medium_padding')
    layout.setContentsMargins(margin, margin, margin, margin)
    layout.setSpacing(style_mgr.get_scaled_size_param('medium_padding'))


class PadsynthProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'PADsynth'

    def __init__(self):
        super().__init__()

        playback_params = PlaybackParams()
        apply_button = ApplyButton()
        sample_config = SampleConfigEditor()
        tabs = PadsynthTabs()

        self.add_to_updaters(playback_params, apply_button, sample_config, tabs)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(playback_params)
        v.addWidget(apply_button)
        v.addWidget(sample_config)
        v.addWidget(tabs)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        _update_layout(self._ui_model.get_style_manager(), self.layout())


class PadsynthTabs(QTabWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        harmonics = Harmonics()
        phases = Phases()
        resonance = Resonance()

        self.add_to_updaters(harmonics, phases, resonance)

        self.addTab(harmonics, 'Harmonics')
        self.addTab(phases, 'Phases')
        self.addTab(resonance, 'Resonance')


class Harmonics(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        bandwidth = BandwidthEditor()
        harmonics_base = HarmonicsBaseEditor()
        harmonic_levels = HarmonicLevels()

        self.add_to_updaters(bandwidth, harmonics_base, harmonic_levels)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(bandwidth)
        v.addWidget(harmonics_base)
        v.addWidget(harmonic_levels)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        _update_layout(self._ui_model.get_style_manager(), self.layout())


class Phases(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._use_button = QCheckBox('Use harmonics phase information')
        self._phase_variation = PhaseVariation()

        self.add_to_updaters(self._phase_variation)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._use_button)
        v.addWidget(self._phase_variation)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self.register_action(self._get_update_signal_type(), self._update_use_phase)

        self._use_button.stateChanged.connect(self._change_use_phase)

        self._update_style()
        self._update_use_phase()

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _update_style(self):
        _update_layout(self._ui_model.get_style_manager(), self.layout())

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_use_phase(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        use = params.get_use_phase_data()

        old_block = self._use_button.blockSignals(True)
        self._use_button.setCheckState(Qt.Checked if use else Qt.Unchecked)
        self._use_button.blockSignals(old_block)

        self._phase_variation.setEnabled(use)

    def _change_use_phase(self, state):
        use = (state == Qt.Checked)
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        params.set_use_phase_data(use)
        self._updater.signal_update(self._get_update_signal_type())


class PhaseVariation(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        var_at_harmonic = PhaseVarAtHarmonic()
        var_off_harmonic = PhaseVarOffHarmonic()
        phase_spread_bw_base = PhaseSpreadBwBaseEditor()
        phase_spread_bw_scale = PhaseSpreadBwScaleEditor()

        self.add_to_updaters(
                var_at_harmonic,
                var_off_harmonic,
                phase_spread_bw_base,
                phase_spread_bw_scale)

        self._var_grid_layout = QGridLayout()
        self._var_grid_layout.setContentsMargins(0, 0, 0, 0)
        self._var_grid_layout.setHorizontalSpacing(0)
        self._var_grid_layout.setVerticalSpacing(0)
        self._var_grid_layout.addWidget(QLabel('At harmonic:'), 0, 0)
        self._var_grid_layout.addWidget(var_at_harmonic, 0, 1)
        self._var_grid_layout.addWidget(QLabel('Off harmonic:'), 1, 0)
        self._var_grid_layout.addWidget(var_off_harmonic, 1, 1)

        self._var_layout = QVBoxLayout()
        self._var_layout.setContentsMargins(0, 0, 0, 0)
        self._var_layout.setSpacing(0)
        self._var_layout.addWidget(HeaderLine('Phase variation'))
        self._var_layout.addLayout(self._var_grid_layout)

        self._spread_grid_layout = QGridLayout()
        self._spread_grid_layout.setContentsMargins(0, 0, 0, 0)
        self._spread_grid_layout.setHorizontalSpacing(0)
        self._spread_grid_layout.setVerticalSpacing(0)
        self._spread_grid_layout.addWidget(QLabel('Bandwidth base:'), 0, 0)
        self._spread_grid_layout.addWidget(phase_spread_bw_base, 0, 1)
        self._spread_grid_layout.addWidget(QLabel('Bandwidth scale:'), 1, 0)
        self._spread_grid_layout.addWidget(phase_spread_bw_scale, 1, 1)

        self._spread_layout = QVBoxLayout()
        self._spread_layout.setContentsMargins(0, 0, 0, 0)
        self._spread_layout.setSpacing(0)
        self._spread_layout.addWidget(HeaderLine('Phase spread'))
        self._spread_layout.addLayout(self._spread_grid_layout)

        self._div_layout = QHBoxLayout()
        self._div_layout.setContentsMargins(0, 0, 0, 0)
        self._div_layout.setSpacing(0)
        self._div_layout.addLayout(self._var_layout)
        self._div_layout.addLayout(self._spread_layout)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addLayout(self._div_layout)
        v.addStretch(1)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        _update_layout(style_mgr, self.layout())

        large_spacing = style_mgr.get_scaled_size_param('large_padding')
        medium_spacing = style_mgr.get_scaled_size_param('medium_padding')
        small_spacing = style_mgr.get_scaled_size_param('small_padding')

        self._div_layout.setSpacing(large_spacing)

        self._var_layout.setSpacing(medium_spacing)
        self._var_grid_layout.setHorizontalSpacing(large_spacing)
        self._var_grid_layout.setVerticalSpacing(small_spacing)

        self._spread_layout.setSpacing(medium_spacing)
        self._spread_grid_layout.setHorizontalSpacing(large_spacing)
        self._spread_grid_layout.setVerticalSpacing(small_spacing)


class PadsynthParamSlider(ProcNumSlider):

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)


class PhaseVarAtHarmonic(PadsynthParamSlider):

    def __init__(self):
        super().__init__(3, 0.0, 1.0)

    def _update_value(self):
        self.set_number(self._get_params().get_phase_var_at_harmonic())

    def _value_changed(self, norm):
        self._get_params().set_phase_var_at_harmonic(norm)
        self._updater.signal_update(self._get_update_signal_type())


class PhaseVarOffHarmonic(PadsynthParamSlider):

    def __init__(self):
        super().__init__(3, 0.0, 1.0)

    def _update_value(self):
        self.set_number(self._get_params().get_phase_var_off_harmonic())

    def _value_changed(self, norm):
        self._get_params().set_phase_var_off_harmonic(norm)
        self._updater.signal_update(self._get_update_signal_type())


class PhaseSpreadBwBaseEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(2, 0.01, 1200.0, width_txt='1200.00')

    def _update_value(self):
        self.set_number(self._get_params().get_phase_spread_bw_base())

    def _value_changed(self, cents):
        self._get_params().set_phase_spread_bw_base(cents)
        self._updater.signal_update(self._get_update_signal_type())


class PhaseSpreadBwScaleEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(2, 0.0, 10.0, width_txt='1200.00')

    def _update_value(self):
        self.set_number(self._get_params().get_phase_spread_bw_scale())

    def _value_changed(self, scale):
        self._get_params().set_phase_spread_bw_scale(scale)
        self._updater.signal_update(self._get_update_signal_type())


class Resonance(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        res_env = ResonanceEnvelope()

        self.add_to_updaters(res_env)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(res_env)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        _update_layout(self._ui_model.get_style_manager(), self.layout())


class PlaybackParams(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._ramp_attack = QCheckBox('Ramp attack')
        self._stereo = QCheckBox('Stereo')
        self._start_pos = StartPositionEditor()

        self.add_to_updaters(self._start_pos)

        self._simple_layout = QHBoxLayout()
        self._simple_layout.setContentsMargins(0, 0, 0, 0)
        self._simple_layout.setSpacing(4)
        self._simple_layout.addWidget(self._ramp_attack)
        self._simple_layout.addWidget(self._stereo)
        self._simple_layout.addStretch(1)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addLayout(self._simple_layout)
        v.addWidget(self._start_pos)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_au', self._update_all)
        self.register_action(self._get_update_signal_type(), self._update_all)
        self.register_action('signal_style_changed', self._update_style)

        self._ramp_attack.stateChanged.connect(self._toggle_ramp_attack)
        self._stereo.stateChanged.connect(self._toggle_stereo)

        self._update_style()
        self._update_all()

    def _get_update_signal_type(self):
        return 'signal_padsynth_rt_{}'.format(self._proc_id)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._simple_layout.setSpacing(style_mgr.get_scaled_size_param('large_padding'))
        self.layout().setSpacing(style_mgr.get_scaled_size_param('medium_padding'))

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
        self._updater.signal_update(self._get_update_signal_type())

    def _toggle_stereo(self, state):
        enabled = (state == Qt.Checked)
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        params.set_stereo_enabled(enabled)
        self._updater.signal_update(self._get_update_signal_type())


class StartPositionEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()

        self._start_pos = StartPosition()
        self._enable_start_pos_var = QCheckBox('Enable start position variation:')
        self._start_pos_var = StartPosVariation()
        self._round_start_pos_var = QCheckBox('Retain phase')

        self.add_to_updaters(self._start_pos, self._start_pos_var)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(0)
        h.addWidget(self._start_pos)
        h.addWidget(self._enable_start_pos_var)
        h.addWidget(self._start_pos_var)
        h.addWidget(self._round_start_pos_var)
        self.setLayout(h)

    def _get_update_signal_type(self):
        return 'signal_padsynth_rt_{}'.format(self._proc_id)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self.register_action(self._get_update_signal_type(), self._update_toggles)

        self._enable_start_pos_var.stateChanged.connect(self._on_var_enabled_changed)
        self._round_start_pos_var.stateChanged.connect(self._on_round_var_changed)

        self._update_style()
        self._update_toggles()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('large_padding'))

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_toggles(self):
        is_var_enabled = self._get_params().get_start_pos_var_enabled()

        old_block = self._enable_start_pos_var.blockSignals(True)
        self._enable_start_pos_var.setCheckState(
                Qt.Checked if is_var_enabled else Qt.Unchecked)
        self._enable_start_pos_var.blockSignals(old_block)

        round_start_pos_var = self._get_params().get_round_start_pos_var()

        old_block = self._round_start_pos_var.blockSignals(True)
        self._round_start_pos_var.setCheckState(
                Qt.Checked if round_start_pos_var else Qt.Unchecked)
        self._round_start_pos_var.blockSignals(old_block)

        self._start_pos_var.setEnabled(is_var_enabled)
        self._round_start_pos_var.setEnabled(is_var_enabled)

    def _on_var_enabled_changed(self, state):
        enabled = (state == Qt.Checked)
        self._get_params().set_start_pos_var_enabled(enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _on_round_var_changed(self, state):
        enabled = (state == Qt.Checked)
        self._get_params().set_round_start_pos_var(enabled)
        self._updater.signal_update(self._get_update_signal_type())


class PadsynthRtParamSlider(ProcNumSlider):

    def _get_update_signal_type(self):
        return 'signal_padsynth_rt_{}'.format(self._proc_id)

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)


class StartPosition(PadsynthRtParamSlider):

    def __init__(self):
        super().__init__(6, 0.0, 1.0, title='Start position:')

    def _update_value(self):
        self.set_number(self._get_params().get_start_pos())

    def _value_changed(self, start_pos):
        self._get_params().set_start_pos(start_pos)
        self._updater.signal_update(self._get_update_signal_type())


class StartPosVariation(PadsynthRtParamSlider):

    def __init__(self):
        super().__init__(6, 0.0, 1.0)

    def _update_value(self):
        self.set_number(self._get_params().get_start_pos_var())

    def _value_changed(self, start_pos_var):
        self._get_params().set_start_pos_var(start_pos_var)
        self._updater.signal_update(self._get_update_signal_type())


class ApplyButton(QPushButton, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._style_creator = StyleCreator()
        self._style_sheet = ''

        self.setText('Apply parameters')
        self.setEnabled(False)

    def _on_setup(self):
        self._style_creator.set_ui_model(self._ui_model)

        self.register_action('signal_au', self._update_status)
        self.register_action(self._get_update_signal_type(), self._update_status)
        self.register_action('signal_style_changed', self._update_style)

        self.clicked.connect(self._apply_params)

        self._style_sheet = QApplication.instance().styleSheet()
        self._update_status()

    def _on_teardown(self):
        self._style_creator.unregister_updaters()

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _update_style(self):
        self._style_sheet = self._style_creator.get_updated_style_sheet()
        self.setStyleSheet(self._style_sheet)

    def _update_status(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        self.setEnabled(not params.is_config_applied())

        self.setObjectName('Important' if self.isEnabled() else '')
        self.setStyleSheet(self._style_sheet)

    def _apply_params(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        def on_complete():
            self._updater.signal_update(self._get_update_signal_type())

        task_executor = self._ui_model.get_task_executor()
        task = params.get_task_apply_config(on_complete)
        task_executor(task)


class SampleConfigEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._sample_size = KqtComboBox()
        self._sample_count = QSpinBox()
        self._sample_count.setRange(1, 128)
        self._range_min = SamplePitchRangeMinEditor()
        self._range_max = SamplePitchRangeMaxEditor()
        self._centre_pitch = SampleCentrePitchEditor()

        self.add_to_updaters(self._range_min, self._range_max, self._centre_pitch)

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
        h.addWidget(QLabel('Centre pitch:'))
        h.addWidget(self._centre_pitch)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_au', self._update_sample_params)
        self.register_action(self._get_update_signal_type(), self._update_sample_params)
        self.register_action('signal_style_changed', self._update_style)

        for sample_length in self._get_params().get_allowed_sample_lengths():
            self._sample_size.addItem(str(sample_length), userData=sample_length)

        self._sample_size.currentIndexChanged.connect(self._change_sample_size)

        self._sample_count.valueChanged.connect(self._change_sample_count)

        self._update_style()
        self._update_sample_params()

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

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
        self._updater.signal_update(self._get_update_signal_type())

    def _change_sample_count(self, count):
        params = self._get_params()
        params.set_sample_count(count)
        self._updater.signal_update(self._get_update_signal_type())


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
        self._updater.signal_update(self._get_update_signal_type())


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
        self._updater.signal_update(self._get_update_signal_type())


class SampleCentrePitchEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(0, -6000.0, 6000.0)

    def _update_value(self):
        centre_pitch = self._get_params().get_sample_centre_pitch()
        self.set_number(centre_pitch)

    def _value_changed(self, centre_pitch):
        self._get_params().set_sample_centre_pitch(centre_pitch)
        self._updater.signal_update(self._get_update_signal_type())


class BandwidthEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._base = BandwidthBaseEditor()
        self._scale = BandwidthScaleEditor()

        self.add_to_updaters(self._base, self._scale)

        g = QGridLayout()
        g.setContentsMargins(0, 0, 0, 0)
        g.setHorizontalSpacing(2)
        g.setVerticalSpacing(0)
        g.addWidget(QLabel('Bandwidth base:'), 0, 0)
        g.addWidget(self._base, 0, 1)
        g.addWidget(QLabel('Bandwidth scale:'), 1, 0)
        g.addWidget(self._scale, 1, 1)
        self.setLayout(g)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self.layout().setHorizontalSpacing(
                style_mgr.get_scaled_size_param('small_padding'))


class BandwidthBaseEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(2, 0.01, 1200.0, width_txt='1200.00')

    def _update_value(self):
        self.set_number(self._get_params().get_bandwidth_base())

    def _value_changed(self, bandwidth):
        self._get_params().set_bandwidth_base(bandwidth)
        self._updater.signal_update(self._get_update_signal_type())


class BandwidthScaleEditor(PadsynthParamSlider):

    def __init__(self):
        super().__init__(2, 0.0, 10.0, width_txt='1200.00')

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_value(self):
        self.set_number(self._get_params().get_bandwidth_scale())

    def _value_changed(self, scale):
        self._get_params().set_bandwidth_scale(scale)
        self._updater.signal_update(self._get_update_signal_type())


class HarmonicsBaseEditor(WaveformEditor):

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_base_wave(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = params.get_harmonics_wave()
        return base_wave


class HarmonicLevelsList(EditorList, ProcessorUpdater):

    def __init__(self):
        super().__init__()

    def _on_setup(self):
        self.register_action('signal_au', self._update_all)
        self.register_action(self._get_update_signal_type(), self._update_all)
        self._update_all()

    def _on_teardown(self):
        self.disconnect_widgets()

    def _make_adder_widget(self):
        adder = HarmonicLevelAdder()
        self.add_to_updaters(adder)
        return adder

    def _get_updated_editor_count(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        count = params.get_harmonic_levels().get_count()
        return count

    def _make_editor_widget(self, index):
        editor = HarmonicLevelEditor(index)
        self.add_to_updaters(editor)
        return editor

    def _update_editor(self, index, editor):
        editor.update_index(index)

    def _disconnect_widget(self, widget):
        self.remove_from_updaters(widget)

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _update_all(self):
        self.update_list()


class HarmonicLevelAdder(QPushButton, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self.setText('Add harmonic level')

    def _on_setup(self):
        self.clicked.connect(self._add_harmonic)

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _add_harmonic(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        levels = params.get_harmonic_levels()
        levels.append_level()
        self._updater.signal_update(self._get_update_signal_type())


class HarmonicLevelEditor(QWidget, ProcessorUpdater):

    def __init__(self, index):
        super().__init__()
        self._index = index

        self._pitch_factor = VarPrecSpinBox(step_decimals=0, max_decimals=4)
        self._pitch_factor.setRange(0.0001, 1024.0)
        self._pitch_factor.setValue(1)

        self._level = LevelEditor(index)

        self._remove_button = ProcessorIconButton()
        self._remove_button.setEnabled(self._index != 0)

        self.add_to_updaters(self._remove_button)

        h = QHBoxLayout()
        h.setContentsMargins(4, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(QLabel('Pitch factor:'))
        h.addWidget(self._pitch_factor)
        h.addWidget(self._level)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def _on_setup(self):
        self.add_to_updaters(self._level)

        self.register_action('signal_style_changed', self._update_style)

        self._remove_button.set_icon('delete_small')

        style_mgr = self._ui_model.get_style_manager()
        self._remove_button.set_sizes(
                style_mgr.get_style_param('list_button_size'),
                style_mgr.get_style_param('list_button_padding'))

        self._pitch_factor.valueChanged.connect(self._change_pitch_factor)
        self._remove_button.clicked.connect(self._remove_harmonic)

        self._update_style()

        self.update_index(self._index)

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()
        self._pitch_factor.update_style(style_mgr)
        self.layout().setContentsMargins(
                style_mgr.get_scaled_size_param('medium_padding'), 0, 0, 0)
        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))

    def update_index(self, index):
        self._index = index

        levels = self._get_params().get_harmonic_levels()
        if self._index >= levels.get_count():
            return

        level = levels.get_level(self._index)

        old_block = self._pitch_factor.blockSignals(True)
        new_pitch_factor = level.get_freq_mul()
        if new_pitch_factor != self._pitch_factor.value():
            self._pitch_factor.setValue(new_pitch_factor)
        self._pitch_factor.blockSignals(old_block)

        self._remove_button.setEnabled(levels.get_count() > 1)

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _change_pitch_factor(self, value):
        levels = self._get_params().get_harmonic_levels()
        if self._index >= levels.get_count():
            return

        level = levels.get_level(self._index)
        level.set_freq_mul(value)
        self._updater.signal_update(self._get_update_signal_type())

    def _remove_harmonic(self):
        levels = self._get_params().get_harmonic_levels()
        if self._index >= levels.get_count():
            return

        levels.remove_level(self._index)
        self._updater.signal_update(self._get_update_signal_type())


class LevelEditor(PadsynthParamSlider):

    def __init__(self, index):
        super().__init__(1, -64.0, 24.0, title='Level:')
        self._index = index

    def _update_value(self):
        levels = self._get_params().get_harmonic_levels()
        if self._index >= levels.get_count():
            return

        level = levels.get_level(self._index)
        self.set_number(level.get_level())

    def _value_changed(self, dB):
        levels = self._get_params().get_harmonic_levels()
        if self._index >= levels.get_count():
            return

        level = levels.get_level(self._index)
        level.set_level(dB)
        self._updater.signal_update(self._get_update_signal_type())


class HarmonicLevels(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._editor = HarmonicLevelsList()
        self.add_to_updaters(self._editor)

        self._header = HeaderLine('Harmonic levels')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(self._header)
        v.addWidget(self._editor)
        self.setLayout(v)

    def _on_setup(self):
        self.register_action('signal_style_changed', self._update_style)
        self._update_style()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        self._header.update_style(style_mgr)

        self.layout().setSpacing(style_mgr.get_scaled_size_param('small_padding'))


class ResonanceEnvelope(ProcessorSimpleEnvelope):

    def __init__(self):
        super().__init__()

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_title(self):
        return 'Resonance envelope'

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _make_envelope_widget(self):
        envelope = Envelope()
        ev = envelope.get_envelope_view()
        ev.set_node_count_max(64)
        ev.set_y_range(0, 1)
        ev.set_y_range_adjust(False, True)
        ev.set_x_range(0, 24000)
        ev.set_first_lock(True, False)
        ev.set_last_lock(True, False)

        return envelope

    def _get_enabled(self):
        return self._get_params().get_resonance_envelope_enabled()

    def _set_enabled(self, enabled):
        self._get_params().set_resonance_envelope_enabled(enabled)

    def _get_envelope_data(self):
        return self._get_params().get_resonance_envelope()

    def _set_envelope_data(self, envelope):
        self._get_params().set_resonance_envelope(envelope)


