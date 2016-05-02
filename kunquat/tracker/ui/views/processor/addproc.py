# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
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
from .waveform import Waveform
from . import utils


class AddProc(QWidget):

    @staticmethod
    def get_name():
        return 'Additive synthesis'

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._ramp_attack = QCheckBox('Ramp attack')
        self._base_waveform = WaveformEditor()
        self._base_tone_editor = ToneList()

        v = QVBoxLayout()
        v.setSpacing(10)
        v.addWidget(self._ramp_attack)
        v.addWidget(self._base_waveform)
        v.addWidget(self._base_tone_editor)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._base_waveform.set_au_id(au_id)
        self._base_tone_editor.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._base_waveform.set_proc_id(proc_id)
        self._base_tone_editor.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._base_waveform.set_ui_model(ui_model)
        self._base_tone_editor.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_ramp_attack()

        QObject.connect(
                self._ramp_attack, SIGNAL('stateChanged(int)'), self._change_ramp_attack)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._base_tone_editor.unregister_updaters()
        self._base_waveform.unregister_updaters()

    def _get_update_signal_type(self):
        return '_'.join(('signal_proc_add_ramp_attack', self._proc_id))

    def _perform_updates(self, signals):
        if self._get_update_signal_type() in signals:
            self._update_ramp_attack()

    def _update_ramp_attack(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        enabled = add_params.get_ramp_attack_enabled()

        old_block = self._ramp_attack.blockSignals(True)
        self._ramp_attack.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self._ramp_attack.blockSignals(old_block)

    def _change_ramp_attack(self, state):
        enabled = (state == Qt.Checked)
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.set_ramp_attack_enabled(enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class WaveformEditor(QWidget):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self._prewarp_list = WarpList('pre')
        self._base_func_selector = QComboBox()
        self._postwarp_list = WarpList('post')
        self._waveform = Waveform()

        pw_layout = QVBoxLayout()
        pw_layout.setSpacing(0)
        pw_layout.addWidget(self._prewarp_list)
        pw_layout.addWidget(self._base_func_selector)
        pw_layout.addWidget(self._postwarp_list)

        ed_layout = QHBoxLayout()
        ed_layout.setSpacing(0)
        ed_layout.addLayout(pw_layout)
        ed_layout.addWidget(self._waveform)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Waveshaping'))
        v.addLayout(ed_layout)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._prewarp_list.set_au_id(au_id)
        self._postwarp_list.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._prewarp_list.set_proc_id(proc_id)
        self._postwarp_list.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._prewarp_list.set_ui_model(ui_model)
        self._postwarp_list.set_ui_model(ui_model)

        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_all()

        QObject.connect(
                self._base_func_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._base_func_selected)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._prewarp_list.unregister_updaters()
        self._postwarp_list.unregister_updaters()

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_', self._au_id, self._proc_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()

        selected_base_func = base_wave.get_waveform_func()
        enable_warps = (selected_base_func != None)

        self._prewarp_list.setEnabled(enable_warps)

        old_block = self._base_func_selector.blockSignals(True)
        self._base_func_selector.clear()
        func_names = base_wave.get_waveform_func_names()
        for i, name in enumerate(func_names):
            self._base_func_selector.addItem(name)
            if name == selected_base_func:
                self._base_func_selector.setCurrentIndex(i)
        if not selected_base_func:
            self._base_func_selector.addItem('Custom')
            self._base_func_selector.setCurrentIndex(len(func_names))
        self._base_func_selector.blockSignals(old_block)

        self._postwarp_list.setEnabled(enable_warps)

        self._waveform.set_waveform(base_wave.get_waveform())

    def _base_func_selected(self, index):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()
        func_names = base_wave.get_waveform_func_names()
        base_wave.set_waveform_func(func_names[index])
        self._updater.signal_update(set([self._get_update_signal_type()]))


class WarpList(EditorList):

    def __init__(self, warp_type):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._icon_bank = None
        self._warp_type = warp_type

        self._adder = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._icon_bank = ui_model.get_icon_bank()
        self._update_all()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

    def _make_adder_widget(self):
        self._adder = WarpAdder(self._warp_type)
        self._adder.set_au_id(self._au_id)
        self._adder.set_proc_id(self._proc_id)
        self._adder.set_ui_model(self._ui_model)
        return self._adder

    def _get_updated_editor_count(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()
        count = base_wave.get_warp_func_count(self._warp_type)

        max_count_reached = (count >= base_wave.get_max_warp_func_count())
        self._adder.setVisible(not max_count_reached)

        return count

    def _make_editor_widget(self, index):
        editor = WarpEditor(self._warp_type, index, self._icon_bank)
        editor.set_au_id(self._au_id)
        editor.set_proc_id(self._proc_id)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_', self._au_id, self._proc_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        self.update_list()

        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()
        warp_count = base_wave.get_warp_func_count(self._warp_type)
        max_count_reached = (warp_count >= base_wave.get_max_warp_func_count())
        self._adder.setVisible(not max_count_reached)


class WarpAdder(QPushButton):

    def __init__(self, warp_type):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._warp_type = warp_type

        self._add_text = { 'pre': 'Add prewarp', 'post': 'Add postwarp' }[warp_type]
        self.setText(self._add_text)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        QObject.connect(self, SIGNAL('clicked()'), self._add_warp)

    def unregister_updaters(self):
        pass

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_', self._au_id, self._proc_id))

    def _add_warp(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()
        base_wave.add_warp_func(self._warp_type)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class SmallButton(QPushButton):

    def __init__(self, icon):
        super().__init__()
        self.setStyleSheet('padding: 0 -2px;')
        self.setIcon(QIcon(icon))


class WarpEditor(QWidget):

    _ARG_SCALE = 1000

    def __init__(self, warp_type, index, icon_bank):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._warp_type = warp_type
        self._index = index

        self._down_button = SmallButton(icon_bank.get_icon_path('arrow_down_small'))
        self._up_button = SmallButton(icon_bank.get_icon_path('arrow_up_small'))
        self._func_selector = QComboBox()
        self._slider = QSlider(Qt.Horizontal)
        self._slider.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)
        self._value_display = QLabel()
        self._remove_button = SmallButton(icon_bank.get_icon_path('delete_small'))

        self._slider.setRange(-self._ARG_SCALE, self._ARG_SCALE)

        self._up_button.setEnabled(self._index != 0)

        fm = QFontMetrics(QFont())
        value_width = fm.boundingRect('{}'.format(-1.0 / self._ARG_SCALE)).width()
        value_width += 10
        self._value_display.setFixedWidth(value_width)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(self._down_button)
        h.addWidget(self._up_button)
        h.addWidget(self._func_selector)
        h.addWidget(self._slider)
        h.addWidget(self._value_display)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()

        func_names = base_wave.get_warp_func_names(self._warp_type)
        old_block = self._func_selector.blockSignals(True)
        self._func_selector.clear()
        for name in func_names:
            self._func_selector.addItem(name)
        self._func_selector.blockSignals(old_block)

        self._update_all()

        QObject.connect(self._down_button, SIGNAL('clicked()'), self._moved_down)
        QObject.connect(self._up_button, SIGNAL('clicked()'), self._moved_up)
        QObject.connect(
                self._func_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._func_selected)
        QObject.connect(self._slider, SIGNAL('valueChanged(int)'), self._slider_adjusted)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._removed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_', self._au_id, self._proc_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()

        warp_func_count = base_wave.get_warp_func_count(self._warp_type)

        if self._index >= warp_func_count:
            # We have been removed
            return

        self._down_button.setEnabled(self._index < warp_func_count - 1)

        name, arg = base_wave.get_warp_func(self._warp_type, self._index)

        old_block = self._func_selector.blockSignals(True)
        for i in range(self._func_selector.count()):
            if self._func_selector.itemText(i) == name:
                self._func_selector.setCurrentIndex(i)
                break
        self._func_selector.blockSignals(old_block)

        old_block = self._slider.blockSignals(True)
        int_val = int(round(arg * self._ARG_SCALE))
        self._slider.setValue(int_val)
        self._slider.blockSignals(old_block)

        self._value_display.setText(str(float(arg)))

    def _moved_down(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()
        base_wave.move_warp_func(self._warp_type, self._index, self._index + 1)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _moved_up(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()
        base_wave.move_warp_func(self._warp_type, self._index, self._index - 1)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _set_warp(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()
        name = str(self._func_selector.currentText())
        arg = self._slider.value() / float(self._ARG_SCALE)
        base_wave.set_warp_func(self._warp_type, self._index, name, arg)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _func_selected(self, index):
        self._set_warp()

    def _slider_adjusted(self, int_val):
        self._set_warp()

    def _removed(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        base_wave = add_params.get_base_wave()
        base_wave.remove_warp_func(self._warp_type, self._index)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class ToneList(EditorList):

    def __init__(self):
        super().__init__()

        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._icon_bank = None

        self._adder = None

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._icon_bank = ui_model.get_icon_bank()
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all()

    def unregister_updaters(self):
        self.disconnect_widgets()
        self._updater.unregister_updater(self._perform_updates)

    def _make_adder_widget(self):
        self._adder = ToneAdder()
        self._adder.set_au_id(self._au_id)
        self._adder.set_proc_id(self._proc_id)
        self._adder.set_ui_model(self._ui_model)
        return self._adder

    def _get_updated_editor_count(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        count = add_params.get_tone_count()
        return count

    def _make_editor_widget(self, index):
        editor = ToneEditor(index, self._icon_bank)
        editor.set_au_id(self._au_id)
        editor.set_proc_id(self._proc_id)
        editor.set_ui_model(self._ui_model)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        widget.unregister_updaters()

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        self.update_list()

        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        count = add_params.get_tone_count()
        max_count_reached = (count >= add_params.get_max_tone_count())
        self._adder.setVisible(not max_count_reached)


class ToneAdder(QPushButton):

    def __init__(self):
        super().__init__('Add tone')
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

        QObject.connect(self, SIGNAL('clicked()'), self._add_tone)

    def unregister_updaters(self):
        pass

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _add_tone(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.add_tone()
        self._updater.signal_update(set([self._get_update_signal_type()]))


class ToneEditor(QWidget):

    _ARG_SCALE = 1000

    def __init__(self, index, icon_bank):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._index = index

        self._pitch_spin = TonePitchSpin(index)
        self._volume_slider = ToneVolumeSlider(index)
        self._panning_slider = TonePanningSlider(index)
        self._remove_button = SmallButton(icon_bank.get_icon_path('delete_small'))

        self._remove_button.setEnabled(self._index != 0)

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(2)
        h.addWidget(self._pitch_spin)
        h.addWidget(self._volume_slider)
        h.addWidget(self._panning_slider)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._pitch_spin.set_au_id(au_id)
        self._volume_slider.set_au_id(au_id)
        self._panning_slider.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._pitch_spin.set_proc_id(proc_id)
        self._volume_slider.set_proc_id(proc_id)
        self._panning_slider.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._pitch_spin.set_ui_model(ui_model)
        self._volume_slider.set_ui_model(ui_model)
        self._panning_slider.set_ui_model(ui_model)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._removed)

    def unregister_updaters(self):
        self._panning_slider.unregister_updaters()
        self._volume_slider.unregister_updaters()
        self._pitch_spin.unregister_updaters()

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _removed(self):
        add_params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        add_params.remove_tone(self._index)
        updater = self._ui_model.get_updater()
        updater.signal_update(set([self._get_update_signal_type()]))


class TonePitchSpin(QWidget):

    def __init__(self, index):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
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

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_value()

        QObject.connect(self._spin, SIGNAL('valueChanged(double)'), self._value_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_value()

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
        self._updater.signal_update(set([self._get_update_signal_type()]))


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
        self._updater.signal_update(set([self._get_update_signal_type()]))

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
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))


