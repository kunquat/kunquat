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

from procnumslider import ProcNumSlider
from waveform import Waveform
from kunquat.tracker.ui.views.headerline import HeaderLine


def get_add_params(obj):
    module = obj._ui_model.get_module()
    au = module.get_audio_unit(obj._au_id)
    proc = au.get_processor(obj._proc_id)
    add_params = proc.get_type_params()
    return add_params


class AddProc(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None

        self._base_waveform = WaveformEditor('base')
        self._base_tone_editor = ToneList('base')

        v = QVBoxLayout()
        v.setSpacing(10)
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

    def get_name(self):
        return 'Additive synthesis'

    def unregister_updaters(self):
        self._base_tone_editor.unregister_updaters()
        self._base_waveform.unregister_updaters()


class WaveformEditor(QWidget):

    def __init__(self, wave_type):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
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

        ed_layout = QHBoxLayout()
        ed_layout.setSpacing(0)
        ed_layout.addLayout(pw_layout)
        ed_layout.addWidget(self._waveform)

        v = QVBoxLayout()
        v.setMargin(0)
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
                ('signal_proc_add_', self._au_id, self._proc_id, self._wave_type))

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
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
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._wave_type = wave_type
        self._warp_type = warp_type
        self._add_text = { 'pre': 'Add prewarp', 'post': 'Add postwarp' }[warp_type]

        self._init_container()

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)

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
                ('signal_proc_add_', self._au_id, self._proc_id, self._wave_type))

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
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
            editor.set_au_id(self._au_id)
            editor.set_proc_id(self._proc_id)
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
        self._au_id = None
        self._proc_id = None
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

    def set_au_id(self, au_id):
        self._au_id = au_id

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

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
                ('signal_proc_add_', self._au_id, self._proc_id, self._wave_type))

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
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


class ToneListArea(QScrollArea):

    def __init__(self):
        QScrollArea.__init__(self)

    def do_width_hack(self):
        self.widget().setFixedWidth(
                self.width() - self.verticalScrollBar().width() - 5)

    def resizeEvent(self, event):
        self.do_width_hack()


class ToneList(QWidget):

    def __init__(self, wave_type):
        QWidget.__init__(self)

        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None
        self._icon_bank = None
        self._wave_type = wave_type

        self._area = ToneListArea()

        self._init_container()

        self._area.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self._area.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(2)
        v.addWidget(HeaderLine('Tones'))
        v.addWidget(self._area)
        self.setLayout(v)

    def _init_container(self):
        self._area.setWidget(ToneListContainer())
        add_button = QPushButton('Add tone')
        QObject.connect(add_button, SIGNAL('clicked()'), self._tone_added)
        self._area.widget().layout().addWidget(add_button)

    def _disconnect_editors(self):
        layout = self._area.widget().layout()
        for i in xrange(layout.count() - 1):
            editor = layout.itemAt(i).widget()
            editor.unregister_updaters()

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
        self._disconnect_editors()
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_au', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _update_all(self):
        add_params = get_add_params(self)

        # Set tone count
        layout = self._area.widget().layout()
        count = add_params.get_tone_count(self._wave_type)
        if count < layout.count() - 1:
            self._disconnect_editors()
            self._init_container()
            layout = self._area.widget().layout()

        # Create new tone editors
        for i in xrange(layout.count() - 1, count):
            editor = ToneEditor(self._wave_type, i, self._icon_bank)
            editor.set_au_id(self._au_id)
            editor.set_proc_id(self._proc_id)
            editor.set_ui_model(self._ui_model)
            layout.insertWidget(i, editor)

        max_count_reached = (count >= add_params.get_max_tone_count())
        layout.itemAt(layout.count() - 1).widget().setVisible(not max_count_reached)

        self._area.do_width_hack()

    def _tone_added(self):
        add_params = get_add_params(self)
        add_params.add_tone(self._wave_type)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def resizeEvent(self, event):
        self._area.do_width_hack()


class ToneEditor(QWidget):

    _ARG_SCALE = 1000

    def __init__(self, wave_type, index, icon_bank):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
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

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._pitch_spin.set_au_id(au_id)
        self._volume_slider.set_au_id(au_id)
        if self._wave_type == 'base':
            self._panning_slider.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._pitch_spin.set_proc_id(proc_id)
        self._volume_slider.set_proc_id(proc_id)
        if self._wave_type == 'base':
            self._panning_slider.set_proc_id(proc_id)

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
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))

    def _removed(self):
        add_params = get_add_params(self)
        add_params.remove_tone(self._wave_type, self._index)
        updater = self._ui_model.get_updater()
        updater.signal_update(set([self._get_update_signal_type()]))


class TonePitchSpin(QWidget):

    def __init__(self, wave_type, index):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
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


class ToneVolumeSlider(ProcNumSlider):

    def __init__(self, wave_type, index):
        ProcNumSlider.__init__(self, 1, -64.0, 24.0, title='Volume')
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
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))


class TonePanningSlider(ProcNumSlider):

    def __init__(self, wave_type, index):
        ProcNumSlider.__init__(self, 3, -1.0, 1.0, title='Panning')
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
        return ''.join(('signal_proc_add_tone_', self._au_id, self._proc_id))


