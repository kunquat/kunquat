# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from gennumslider import GenNumSlider
from waveform import Waveform
from kunquat.tracker.ui.views.envelope import Envelope
from kunquat.tracker.ui.views.numberslider import NumberSlider
from kunquat.tracker.ui.views.instrument.time_env import TimeEnvelope


def get_add_params(obj):
    module = obj._ui_model.get_module()
    instrument = module.get_instrument(obj._ins_id)
    generator = instrument.get_generator(obj._gen_id)
    add_params = generator.get_type_params()
    return add_params


class AddGen(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None

        self._base_waveform = WaveformEditor('base')
        self._base_tone_editor = ToneList('base')

        base_layout = QHBoxLayout()
        base_layout.setSpacing(0)
        base_layout.addWidget(self._base_waveform)
        base_layout.addWidget(self._base_tone_editor)

        self._phase_mod_enabled_toggle = QCheckBox('Phase modulation')
        self._phase_mod_volume = ModVolume()
        self._phase_mod_env = ModEnv()
        self._phase_force_mod_env = ForceModEnv()
        self._phase_mod_waveform = WaveformEditor('mod')
        self._phase_mod_tone_editor = ToneList('mod')

        pmenv_layout = QHBoxLayout()
        pmenv_layout.setSpacing(5)
        pmenv_layout.addWidget(self._phase_mod_env, 5)
        pmenv_layout.addWidget(self._phase_force_mod_env, 2)

        pmvol_layout = QVBoxLayout()
        pmvol_layout.setSpacing(0)
        pmvol_layout.addWidget(self._phase_mod_volume)
        pmvol_layout.addLayout(pmenv_layout)

        pmwave_layout = QHBoxLayout()
        pmwave_layout.setSpacing(0)
        pmwave_layout.addWidget(self._phase_mod_waveform)
        pmwave_layout.addWidget(self._phase_mod_tone_editor)

        self._phase_mod_container = QWidget()
        pmc_layout = QVBoxLayout()
        pmc_layout.setSpacing(2)
        pmc_layout.addLayout(pmvol_layout, 5)
        pmc_layout.addLayout(pmwave_layout, 2)
        self._phase_mod_container.setLayout(pmc_layout)

        mod_layout = QVBoxLayout()
        mod_layout.setSpacing(0)
        mod_layout.addWidget(self._phase_mod_enabled_toggle)
        mod_layout.addWidget(self._phase_mod_container)

        h = QVBoxLayout()
        h.setSpacing(5)
        h.addLayout(base_layout, 1)
        h.addLayout(mod_layout, 3)
        self.setLayout(h)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id
        self._base_waveform.set_ins_id(ins_id)
        self._base_tone_editor.set_ins_id(ins_id)
        self._phase_mod_volume.set_ins_id(ins_id)
        self._phase_mod_env.set_ins_id(ins_id)
        self._phase_force_mod_env.set_ins_id(ins_id)
        self._phase_mod_waveform.set_ins_id(ins_id)
        self._phase_mod_tone_editor.set_ins_id(ins_id)

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id
        self._base_waveform.set_gen_id(gen_id)
        self._base_tone_editor.set_gen_id(gen_id)
        self._phase_mod_volume.set_gen_id(gen_id)
        self._phase_mod_env.set_gen_id(gen_id)
        self._phase_force_mod_env.set_gen_id(gen_id)
        self._phase_mod_waveform.set_gen_id(gen_id)
        self._phase_mod_tone_editor.set_gen_id(gen_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._base_waveform.set_ui_model(ui_model)
        self._base_tone_editor.set_ui_model(ui_model)
        self._phase_mod_volume.set_ui_model(ui_model)
        self._phase_mod_env.set_ui_model(ui_model)
        self._phase_force_mod_env.set_ui_model(ui_model)
        self._phase_mod_waveform.set_ui_model(ui_model)
        self._phase_mod_tone_editor.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_gen()

        QObject.connect(
                self._phase_mod_enabled_toggle,
                SIGNAL('stateChanged(int)'),
                self._phase_mod_enabled_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._phase_mod_tone_editor.unregister_updaters()
        self._phase_mod_waveform.unregister_updaters()
        self._phase_force_mod_env.unregister_updaters()
        self._phase_mod_env.unregister_updaters()
        self._phase_mod_volume.unregister_updaters()
        self._base_tone_editor.unregister_updaters()
        self._base_waveform.unregister_updaters()

    def _get_update_signal_type(self):
        return ''.join(('signal_gen_add_', self._ins_id, self._gen_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_gen()

    def _update_gen(self):
        add_params = get_add_params(self)
        phase_mod_enabled = add_params.get_phase_mod_enabled()

        old_block = self._phase_mod_enabled_toggle.blockSignals(True)
        self._phase_mod_enabled_toggle.setCheckState(
                Qt.Checked if phase_mod_enabled else Qt.Unchecked)
        self._phase_mod_enabled_toggle.blockSignals(old_block)

        self._phase_mod_container.setEnabled(phase_mod_enabled)

    def _phase_mod_enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        add_params = get_add_params(self)
        add_params.set_phase_mod_enabled(new_enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class WaveformEditor(QWidget):

    def __init__(self, wave_type):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None
        self._wave_type = wave_type

        self._prewarp_list = WarpList(self._wave_type, 'pre')
        self._base_func_selector = QComboBox()
        self._postwarp_list = WarpList(self._wave_type, 'post')
        self._waveform = Waveform()

        pw_layout = QVBoxLayout()
        pw_layout.setSpacing(0)
        pw_layout.addWidget(self._prewarp_list)
        pw_layout.addWidget(self._base_func_selector)
        pw_layout.addWidget(self._postwarp_list)

        v = QHBoxLayout()
        v.setSpacing(0)
        v.addLayout(pw_layout)
        v.addWidget(self._waveform)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id
        self._prewarp_list.set_ins_id(ins_id)
        self._postwarp_list.set_ins_id(ins_id)

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id
        self._prewarp_list.set_gen_id(gen_id)
        self._postwarp_list.set_gen_id(gen_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._prewarp_list.set_ui_model(ui_model)
        self._postwarp_list.set_ui_model(ui_model)

        add_params = get_add_params(self)

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
        return ''.join(
                ('signal_gen_add_', self._ins_id, self._gen_id, self._wave_type))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        add_params = get_add_params(self)

        selected_base_func = add_params.get_waveform_func(self._wave_type)
        enable_warps = (selected_base_func != None)

        self._prewarp_list.setEnabled(enable_warps)

        old_block = self._base_func_selector.blockSignals(True)
        self._base_func_selector.clear()
        func_names = add_params.get_waveform_func_names(self._wave_type)
        for i, name in enumerate(func_names):
            self._base_func_selector.addItem(name)
            if name == selected_base_func:
                self._base_func_selector.setCurrentIndex(i)
        if not selected_base_func:
            self._base_func_selector.addItem('Custom')
            self._base_func_selector.setCurrentIndex(len(func_names))
        self._base_func_selector.blockSignals(old_block)

        self._postwarp_list.setEnabled(enable_warps)

        self._waveform.set_waveform(add_params.get_waveform(self._wave_type))

    def _base_func_selected(self, index):
        add_params = get_add_params(self)
        func_names = add_params.get_waveform_func_names(self._wave_type)
        add_params.set_waveform_func(self._wave_type, func_names[index])
        self._updater.signal_update(set([self._get_update_signal_type()]))


class WarpListContainer(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.setSizeConstraint(QLayout.SetMinimumSize)
        self.setLayout(v)


class WarpList(QScrollArea):

    def __init__(self, wave_type, warp_type):
        QAbstractScrollArea.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None
        self._wave_type = wave_type
        self._warp_type = warp_type
        self._add_text = { 'pre': 'Add prewarp', 'post': 'Add postwarp' }[warp_type]

        self._init_container()

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._icon_bank = ui_model.get_icon_bank()
        self._update_all()

    def unregister_updaters(self):
        layout = self.widget().layout()
        for i in xrange(layout.count() - 1):
            editor = layout.itemAt(i).widget()
            editor.unregister_updaters()

        self._updater.unregister_updater(self._perform_updates)

    def _init_container(self):
        if self.widget():
            old_layout = self.widget().layout()
            for i in xrange(old_layout.count() - 1):
                editor = old_layout.itemAt(i).widget()
                editor.unregister_updaters()

        self.setWidget(WarpListContainer())
        add_button = QPushButton(self._add_text)
        QObject.connect(add_button, SIGNAL('clicked()'), self._warp_added)
        self.widget().layout().addWidget(add_button)

    def _get_update_signal_type(self):
        return ''.join(
                ('signal_gen_add_', self._ins_id, self._gen_id, self._wave_type))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        add_params = get_add_params(self)

        warp_count = add_params.get_warp_func_count(self._wave_type, self._warp_type)

        layout = self.widget().layout()

        if warp_count < layout.count() - 1:
            # Create contents from scratch because
            # Qt doesn't update visuals properly on single item removal
            self._init_container()
            layout = self.widget().layout()

        # Create new items
        for i in xrange(layout.count() - 1, warp_count):
            editor = WarpEditor(self._wave_type, self._warp_type, i, self._icon_bank)
            editor.set_ins_id(self._ins_id)
            editor.set_gen_id(self._gen_id)
            editor.set_ui_model(self._ui_model)
            layout.insertWidget(i, editor)

        max_count_reached = (warp_count >= add_params.get_max_warp_func_count())
        layout.itemAt(layout.count() - 1).widget().setVisible(not max_count_reached)

        self._do_width_hack()

    def set_warp(self, index, name, arg):
        editor = self.widget().layout().itemAt(index).widget()
        editor.set_warp(name, arg)

    def get_warp(self, index):
        editor = self.widget().layout().itemAt(index).widget()
        return editor.get_warp()

    def _warp_added(self):
        add_params = get_add_params(self)
        add_params.add_warp_func(self._wave_type, self._warp_type)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _do_width_hack(self):
        self.widget().setFixedWidth(
                self.width() - self.verticalScrollBar().width() - 5)

    def resizeEvent(self, event):
        self._do_width_hack()


class SmallButton(QPushButton):

    def __init__(self, icon):
        QPushButton.__init__(self)
        self.setStyleSheet('padding: 0 -2px;')
        self.setIcon(QIcon(icon))


class WarpEditor(QWidget):

    _ARG_SCALE = 1000

    def __init__(self, wave_type, warp_type, index, icon_bank):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None
        self._wave_type = wave_type
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
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._down_button)
        h.addWidget(self._up_button)
        h.addWidget(self._func_selector)
        h.addWidget(self._slider)
        h.addWidget(self._value_display)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        add_params = get_add_params(self)

        func_names = add_params.get_warp_func_names(self._warp_type)
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
        return ''.join(
                ('signal_gen_add_', self._ins_id, self._gen_id, self._wave_type))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        add_params = get_add_params(self)

        warp_func_count = add_params.get_warp_func_count(
                self._wave_type, self._warp_type)

        if self._index >= warp_func_count:
            # We have been removed
            return

        self._down_button.setEnabled(self._index < warp_func_count - 1)

        name, arg = add_params.get_warp_func(
                self._wave_type, self._warp_type, self._index)

        old_block = self._func_selector.blockSignals(True)
        for i in xrange(self._func_selector.count()):
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
        add_params = get_add_params(self)
        add_params.move_warp_func(
                self._wave_type, self._warp_type, self._index, self._index + 1)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _moved_up(self):
        add_params = get_add_params(self)
        add_params.move_warp_func(
                self._wave_type, self._warp_type, self._index, self._index - 1)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _set_warp(self):
        add_params = get_add_params(self)
        name = str(self._func_selector.currentText())
        arg = self._slider.value() / float(self._ARG_SCALE)
        add_params.set_warp_func(
                self._wave_type, self._warp_type, self._index, name, arg)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _func_selected(self, index):
        self._set_warp()

    def _slider_adjusted(self, int_val):
        self._set_warp()

    def _removed(self):
        add_params = get_add_params(self)
        add_params.remove_warp_func(self._wave_type, self._warp_type, self._index)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class ToneListContainer(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.setSizeConstraint(QLayout.SetMinimumSize)
        self.setLayout(v)


class ToneList(QScrollArea):

    def __init__(self, wave_type):
        QScrollArea.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None
        self._icon_bank = None
        self._wave_type = wave_type

        self._init_container()

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

    def _init_container(self):
        self.setWidget(ToneListContainer())
        add_button = QPushButton('Add tone')
        QObject.connect(add_button, SIGNAL('clicked()'), self._tone_added)
        self.widget().layout().addWidget(add_button)

    def _disconnect_editors(self):
        layout = self.widget().layout()
        for i in xrange(layout.count() - 1):
            editor = layout.itemAt(i).widget()
            editor.unregister_updaters()

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._icon_bank = ui_model.get_icon_bank()
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_all()

    def unregister_updaters(self):
        self._disconnect_editors()
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return ''.join(('signal_gen_add_tone_', self._ins_id, self._gen_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        add_params = get_add_params(self)

        # Set tone count
        layout = self.widget().layout()
        count = add_params.get_tone_count(self._wave_type)
        if count < layout.count() - 1:
            self._disconnect_editors()
            self._init_container()
            layout = self.widget().layout()

        # Create new tone editors
        for i in xrange(layout.count() - 1, count):
            editor = ToneEditor(self._wave_type, i, self._icon_bank)
            editor.set_ins_id(self._ins_id)
            editor.set_gen_id(self._gen_id)
            editor.set_ui_model(self._ui_model)
            layout.insertWidget(i, editor)

        max_count_reached = (count >= add_params.get_max_tone_count())
        layout.itemAt(layout.count() - 1).widget().setVisible(not max_count_reached)

        self._do_width_hack()

    def _do_width_hack(self):
        self.widget().setFixedWidth(
                self.width() - self.verticalScrollBar().width() - 5)

    def _tone_added(self):
        add_params = get_add_params(self)
        add_params.add_tone(self._wave_type)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def resizeEvent(self, event):
        self._do_width_hack()


class ToneEditor(QWidget):

    _ARG_SCALE = 1000

    def __init__(self, wave_type, index, icon_bank):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._wave_type = wave_type
        self._index = index

        self._pitch_spin = TonePitchSpin(self._wave_type, index)
        self._volume_slider = ToneVolumeSlider(self._wave_type, index)
        self._panning_slider = TonePanningSlider(self._wave_type, index)
        self._remove_button = SmallButton(icon_bank.get_icon_path('delete_small'))

        self._remove_button.setEnabled(self._index != 0)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(2)
        h.addWidget(self._pitch_spin)
        h.addWidget(self._volume_slider)
        if self._wave_type == 'base':
            h.addWidget(self._panning_slider)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id
        self._pitch_spin.set_ins_id(ins_id)
        self._volume_slider.set_ins_id(ins_id)
        if self._wave_type == 'base':
            self._panning_slider.set_ins_id(ins_id)

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id
        self._pitch_spin.set_gen_id(gen_id)
        self._volume_slider.set_gen_id(gen_id)
        if self._wave_type == 'base':
            self._panning_slider.set_gen_id(gen_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._pitch_spin.set_ui_model(ui_model)
        self._volume_slider.set_ui_model(ui_model)
        if self._wave_type == 'base':
            self._panning_slider.set_ui_model(ui_model)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._removed)

    def unregister_updaters(self):
        if self._wave_type == 'base':
            self._panning_slider.unregister_updaters()
        self._volume_slider.unregister_updaters()
        self._pitch_spin.unregister_updaters()

    def _get_update_signal_type(self):
        return ''.join(('signal_gen_add_tone_', self._ins_id, self._gen_id))

    def _removed(self):
        add_params = get_add_params(self)
        add_params.remove_tone(self._wave_type, self._index)
        updater = self._ui_model.get_updater()
        updater.signal_update(set([self._get_update_signal_type()]))


class TonePitchSpin(QWidget):

    def __init__(self, wave_type, index):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None
        self._wave_type = wave_type
        self._index = index

        self._spin = QDoubleSpinBox()
        self._spin.setDecimals(3)
        self._spin.setMinimum(0.001)
        self._spin.setMaximum(1024.0)
        self._spin.setValue(1)

        h = QHBoxLayout()
        h.setMargin(0)
        h.addWidget(QLabel('Pitch'))
        h.addWidget(self._spin)
        self.setLayout(h)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_value()

        QObject.connect(self._spin, SIGNAL('valueChanged(double)'), self._value_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return ''.join(('signal_gen_add_tone_', self._ins_id, self._gen_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_value()

    def _update_value(self):
        add_params = get_add_params(self)

        if self._index >= add_params.get_tone_count(self._wave_type):
            # We have been removed
            return

        old_block = self._spin.blockSignals(True)
        new_pitch = add_params.get_tone_pitch(self._wave_type, self._index)
        if new_pitch != self._spin.value():
            self._spin.setValue(new_pitch)
        self._spin.blockSignals(old_block)

    def _value_changed(self, pitch):
        add_params = get_add_params(self)
        add_params.set_tone_pitch(self._wave_type, self._index, pitch)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class ToneVolumeSlider(GenNumSlider):

    def __init__(self, wave_type, index):
        GenNumSlider.__init__(self, 1, -64.0, 24.0, title='Volume')
        self._wave_type = wave_type
        self._index = index
        self.set_number(0)

    def _update_value(self):
        add_params = get_add_params(self)

        if self._index >= add_params.get_tone_count(self._wave_type):
            # We have been removed
            return

        self.set_number(add_params.get_tone_volume(self._wave_type, self._index))

    def _value_changed(self, volume):
        add_params = get_add_params(self)
        add_params.set_tone_volume(self._wave_type, self._index, volume)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return ''.join(('signal_gen_add_tone_', self._ins_id, self._gen_id))


class TonePanningSlider(GenNumSlider):

    def __init__(self, wave_type, index):
        GenNumSlider.__init__(self, 3, -1.0, 1.0, title='Panning')
        self._wave_type = wave_type
        self._index = index
        self.set_number(0)

    def _update_value(self):
        add_params = get_add_params(self)

        if self._index >= add_params.get_tone_count(self._wave_type):
            # We have been removed
            return

        self.set_number(add_params.get_tone_panning(self._wave_type, self._index))

    def _value_changed(self, panning):
        add_params = get_add_params(self)
        add_params.set_tone_panning(self._wave_type, self._index, panning)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return ''.join(('signal_gen_add_tone_', self._ins_id, self._gen_id))


class ModVolume(GenNumSlider):

    def __init__(self):
        GenNumSlider.__init__(self, 2, -64.0, 24.0, title='Mod volume')
        self.set_number(0)

    def _update_value(self):
        add_params = get_add_params(self)
        self.set_number(add_params.get_mod_volume())

    def _value_changed(self, mod_volume):
        add_params = get_add_params(self)
        add_params.set_mod_volume(mod_volume)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return ''.join(('signal_add_mod_volume_', self._ins_id, self._gen_id))


class ModEnv(TimeEnvelope):

    def __init__(self):
        TimeEnvelope.__init__(self)
        self._gen_id = None

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def _get_title(self):
        return 'Mod envelope'

    def _allow_loop(self):
        return False

    def _make_envelope_widget(self):
        envelope = Envelope()
        envelope.set_node_count_max(32)
        envelope.set_y_range(0, 1)
        envelope.set_x_range(0, 4)
        envelope.set_first_lock(True, False)
        envelope.set_x_range_adjust(False, True)
        return envelope

    def _get_update_signal_type(self):
        return ''.join(('signal_add_mod_env_', self._ins_id, self._gen_id))

    def _get_enabled(self):
        return get_add_params(self).get_mod_envelope_enabled()

    def _set_enabled(self, enabled):
        get_add_params(self).set_mod_envelope_enabled(enabled)

    def _get_scale_amount(self):
        return get_add_params(self).get_mod_envelope_scale_amount()

    def _set_scale_amount(self, value):
        get_add_params(self).set_mod_envelope_scale_amount(value)

    def _get_scale_center(self):
        return get_add_params(self).get_mod_envelope_scale_center()

    def _set_scale_center(self, value):
        get_add_params(self).set_mod_envelope_scale_center(value)

    def _get_envelope_data(self):
        return get_add_params(self).get_mod_envelope()

    def _set_envelope_data(self, envelope):
        get_add_params(self).set_mod_envelope(envelope)


class ForceModEnv(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None

        header = QLabel(self._get_title())
        header.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)
        header_line = QFrame()
        header_line.setFrameShape(QFrame.HLine)
        header_line.setFrameShadow(QFrame.Sunken)
        header_line.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Maximum)
        header_layout = QHBoxLayout()
        header_layout.setContentsMargins(5, 0, 5, 0)
        header_layout.setSpacing(10)
        header_layout.addWidget(header)
        header_layout.addWidget(header_line)

        self._enabled_toggle = QCheckBox('Enabled')

        self._envelope = self._make_envelope_widget()

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addLayout(header_layout)
        v.addWidget(self._enabled_toggle)
        v.addWidget(self._envelope)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_envelope()

        QObject.connect(
                self._enabled_toggle,
                SIGNAL('stateChanged(int)'),
                self._enabled_changed)
        QObject.connect(
                self._envelope,
                SIGNAL('envelopeChanged()'),
                self._envelope_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return ''.join(('signal_add_force_mod_volume_', self._ins_id, self._gen_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_envelope()

    def _update_envelope(self):
        old_block = self._enabled_toggle.blockSignals(True)
        self._enabled_toggle.setCheckState(
                Qt.Checked if self._get_enabled() else Qt.Unchecked)
        self._enabled_toggle.blockSignals(old_block)

        envelope = self._get_envelope_data()
        self._envelope.set_nodes(envelope['nodes'])

    def _enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        self._set_enabled(new_enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _envelope_changed(self):
        new_nodes, _ = self._envelope.get_clear_changed()

        envelope = self._get_envelope_data()
        if new_nodes:
            envelope['nodes'] = new_nodes

        if new_nodes:
            self._set_enabled(True)

        self._set_envelope_data(envelope)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_title(self):
        return 'Force-mod envelope'

    def _make_envelope_widget(self):
        envelope = Envelope({ 'is_square_area': True })
        envelope.set_node_count_max(32)
        envelope.set_y_range(0, 1)
        envelope.set_x_range(0, 1)
        envelope.set_first_lock(True, False)
        envelope.set_last_lock(True, False)
        return envelope

    def _get_enabled(self):
        add_params = get_add_params(self)
        return add_params.get_force_mod_envelope_enabled()

    def _set_enabled(self, enabled):
        add_params = get_add_params(self)
        add_params.set_force_mod_envelope_enabled(enabled)

    def _get_envelope_data(self):
        add_params = get_add_params(self)
        return add_params.get_force_mod_envelope()

    def _set_envelope_data(self, envelope):
        add_params = get_add_params(self)
        add_params.set_force_mod_envelope(envelope)


