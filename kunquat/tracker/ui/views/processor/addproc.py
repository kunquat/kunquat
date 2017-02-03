# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
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

from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.headerline import HeaderLine
from .procnumslider import ProcNumSlider
from .processorupdater import ProcessorUpdater
from .waveformeditor import WaveformEditor
from . import utils


class AddProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Additive synthesis'

    def __init__(self):
        super().__init__()
        self._ramp_attack = QCheckBox('Ramp attack')
        self._rand_phase = QCheckBox('Random initial phase')
        self._base_waveform = AddWaveformEditor()
        self._base_tone_editor = ToneList()

        self.add_to_updaters(self._base_waveform, self._base_tone_editor)

        h = QHBoxLayout()
        h.setContentsMargins(4, 4, 4, 4)
        h.setSpacing(10)
        h.addWidget(self._ramp_attack)
        h.addWidget(self._rand_phase)
        h.addStretch(1)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(10)
        v.addLayout(h)
        v.addWidget(self._base_waveform)
        v.addWidget(self._base_tone_editor)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_simple_params)

        QObject.connect(
                self._ramp_attack, SIGNAL('stateChanged(int)'), self._change_ramp_attack)
        QObject.connect(
                self._rand_phase, SIGNAL('stateChanged(int)'), self._change_rand_phase)

        self._update_simple_params()

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_add_simple_params', self._proc_id))

    def _update_simple_params(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        # Ramp attack
        ramp_attack_enabled = add_params.get_ramp_attack_enabled()
        old_block = self._ramp_attack.blockSignals(True)
        self._ramp_attack.setCheckState(
                Qt.Checked if ramp_attack_enabled else Qt.Unchecked)
        self._ramp_attack.blockSignals(old_block)

        # Random initial phase
        rand_phase_enabled = add_params.get_rand_phase_enabled()
        old_block = self._rand_phase.blockSignals(True)
        self._rand_phase.setCheckState(
                Qt.Checked if rand_phase_enabled else Qt.Unchecked)
        self._rand_phase.blockSignals(old_block)

    def _change_ramp_attack(self, state):
        enabled = (state == Qt.Checked)
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.set_ramp_attack_enabled(enabled)
        self._updater.signal_update(self._get_update_signal_type())

    def _change_rand_phase(self, state):
        enabled = (state == Qt.Checked)
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.set_rand_phase_enabled(enabled)
        self._updater.signal_update(self._get_update_signal_type())


class AddWaveformEditor(WaveformEditor):

    def _get_update_signal_type(self):
        return 'signal_proc_add_{}'.format(self._proc_id)

    def _get_base_wave(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()
        return base_wave


class SmallButton(QPushButton):

    def __init__(self, icon):
        super().__init__()
        self.setStyleSheet('padding: 0 -2px;')
        self.setIcon(QIcon(icon))


class ToneList(EditorList, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._icon_bank = None
        self._adder = None

    def _on_setup(self):
        self._icon_bank = self._ui_model.get_icon_bank()
        self.register_action('signal_au', self._update_all)
        self.register_action(self._get_update_signal_type(), self._update_all)
        self._update_all()

    def _on_teardown(self):
        self.disconnect_widgets()

    def _make_adder_widget(self):
        self._adder = ToneAdder()
        self.add_to_updaters(self._adder)
        return self._adder

    def _get_updated_editor_count(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        count = add_params.get_tone_count()
        return count

    def _make_editor_widget(self, index):
        editor = ToneEditor(index, self._icon_bank)
        self.add_to_updaters(editor)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        self.remove_from_updaters(widget)

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _update_all(self):
        self.update_list()

        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        count = add_params.get_tone_count()
        max_count_reached = (count >= add_params.get_max_tone_count())
        self._adder.setVisible(not max_count_reached)


class ToneAdder(QPushButton, ProcessorUpdater):

    def __init__(self):
        super().__init__('Add tone')

    def _on_setup(self):
        QObject.connect(self, SIGNAL('clicked()'), self._add_tone)

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _add_tone(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.add_tone()
        self._updater.signal_update(self._get_update_signal_type())


class ToneEditor(QWidget, ProcessorUpdater):

    _ARG_SCALE = 1000

    def __init__(self, index, icon_bank):
        super().__init__()
        self._index = index

        self._pitch_spin = TonePitchSpin(index)
        self._volume_slider = ToneVolumeSlider(index)
        self._panning_slider = TonePanningSlider(index)
        self._remove_button = SmallButton(icon_bank.get_icon_path('delete_small'))

        self.add_to_updaters(
                self._pitch_spin, self._volume_slider, self._panning_slider)

        self._remove_button.setEnabled(self._index != 0)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(self._pitch_spin)
        h.addWidget(self._volume_slider)
        h.addWidget(self._panning_slider)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def _on_setup(self):
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._removed)

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _removed(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.remove_tone(self._index)
        updater = self._ui_model.get_updater()
        updater.signal_update(self._get_update_signal_type())


class TonePitchSpin(QWidget, ProcessorUpdater):

    def __init__(self, index):
        super().__init__()
        self._index = index

        self._spin = QDoubleSpinBox()
        self._spin.setDecimals(3)
        self._spin.setMinimum(0.001)
        self._spin.setMaximum(1024.0)
        self._spin.setValue(1)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.addWidget(QLabel('Pitch'))
        h.addWidget(self._spin)
        self.setLayout(h)

    def _on_setup(self):
        self.register_action('signal_au', self._update_value)
        self.register_action(self._get_update_signal_type(), self._update_value)
        self._update_value()
        QObject.connect(self._spin, SIGNAL('valueChanged(double)'), self._value_changed)

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _update_value(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        if self._index >= add_params.get_tone_count():
            # We have been removed
            return

        old_block = self._spin.blockSignals(True)
        new_pitch = add_params.get_tone_pitch(self._index)
        if new_pitch != self._spin.value():
            self._spin.setValue(new_pitch)
        self._spin.blockSignals(old_block)

    def _value_changed(self, pitch):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.set_tone_pitch(self._index, pitch)
        self._updater.signal_update(self._get_update_signal_type())


class ToneVolumeSlider(ProcNumSlider):

    def __init__(self, index):
        super().__init__(1, -64.0, 24.0, title='Volume')
        self._index = index
        self.set_number(0)

    def _update_value(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        if self._index >= add_params.get_tone_count():
            # We have been removed
            return

        self.set_number(add_params.get_tone_volume(self._index))

    def _value_changed(self, volume):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.set_tone_volume(self._index, volume)
        self._updater.signal_update(self._get_update_signal_type())

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))


class TonePanningSlider(ProcNumSlider):

    def __init__(self, index):
        super().__init__(3, -1.0, 1.0, title='Panning')
        self._index = index
        self.set_number(0)

    def _update_value(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        if self._index >= add_params.get_tone_count():
            # We have been removed
            return

        self.set_number(add_params.get_tone_panning(self._index))

    def _value_changed(self, panning):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.set_tone_panning(self._index, panning)
        self._updater.signal_update(self._get_update_signal_type())

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))


