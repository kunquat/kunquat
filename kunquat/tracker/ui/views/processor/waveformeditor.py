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

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.tracker.ui.views.editorlist import EditorList
from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.kqtcombobox import KqtComboBox
from .processorupdater import ProcessorUpdater
from .waveform import Waveform


class WaveformEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._prewarp_list = WarpList(
                'pre', self._get_base_wave, self._get_update_signal_type)
        self._base_func_selector = KqtComboBox()
        self._postwarp_list = WarpList(
                'post', self._get_base_wave, self._get_update_signal_type)
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

    def _on_setup(self):
        self.add_to_updaters(self._prewarp_list, self._postwarp_list)
        self.register_action('signal_au', self._update_all)
        self.register_action(self._get_update_signal_type(), self._update_all)
        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_all()

        QObject.connect(
                self._base_func_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._base_func_selected)

    def _update_style(self):
        style_manager = self._ui_model.get_style_manager()
        if not style_manager.is_custom_style_enabled():
            self._waveform.set_config({})
            return

        def get_colour(name):
            return QColor(style_manager.get_style_param(name))

        disabled_colour = QColor(get_colour('bg_colour'))
        disabled_colour.setAlpha(0x7f)

        config = {
            'bg_colour': get_colour('waveform_bg_colour'),
            'centre_line_colour': get_colour('waveform_centre_line_colour'),
            'waveform_colour': get_colour('waveform_zoomed_out_colour'),
            'disabled_colour': disabled_colour,
        }

        self._waveform.set_config(config)

    def _update_all(self):
        base_wave = self._get_base_wave()

        selected_base_func = base_wave.get_waveform_func()
        enable_warps = (selected_base_func != None)

        self._prewarp_list.setEnabled(enable_warps)

        old_block = self._base_func_selector.blockSignals(True)
        func_names = list(base_wave.get_waveform_func_names())
        if not selected_base_func:
            func_names.append('Custom')
        self._base_func_selector.set_items(func_names)
        self._base_func_selector.setCurrentIndex(
                self._base_func_selector.findText(selected_base_func or 'Custom'))
        self._base_func_selector.blockSignals(old_block)

        self._postwarp_list.setEnabled(enable_warps)

        self._waveform.set_waveform(base_wave.get_waveform())

    def _base_func_selected(self, index):
        base_wave = self._get_base_wave()
        func_names = base_wave.get_waveform_func_names()
        base_wave.set_waveform_func(func_names[index])
        self._updater.signal_update(self._get_update_signal_type())

    # Protected interface

    def _get_base_wave(self):
        raise NotImplementedError

    def _get_update_signal_type(self):
        raise NotImplementedError


class WarpList(EditorList, ProcessorUpdater):

    def __init__(self, warp_type, get_base_wave, get_update_signal_type):
        super().__init__()
        self._warp_type = warp_type
        self._get_base_wave = get_base_wave
        self._get_update_signal_type = get_update_signal_type

        self._adder = None

    def _on_setup(self):
        self.register_action('signal_au', self._update_all)
        self.register_action(self._get_update_signal_type(), self._update_all)

        self._update_all()

    def _on_teardown(self):
        self.disconnect_widgets()

    def _make_adder_widget(self):
        self._adder = WarpAdder(
                self._warp_type, self._get_base_wave, self._get_update_signal_type)
        self.add_to_updaters(self._adder)
        return self._adder

    def _get_updated_editor_count(self):
        base_wave = self._get_base_wave()
        count = base_wave.get_warp_func_count(self._warp_type)

        max_count_reached = (count >= base_wave.get_max_warp_func_count())
        self._adder.setVisible(not max_count_reached)

        return count

    def _make_editor_widget(self, index):
        editor = WarpEditor(
                self._warp_type,
                index,
                self._get_base_wave,
                self._get_update_signal_type)
        self.add_to_updaters(editor)
        return editor

    def _update_editor(self, index, editor):
        pass

    def _disconnect_widget(self, widget):
        self.remove_from_updaters(widget)

    def _update_all(self):
        self.update_list()

        base_wave = self._get_base_wave()
        warp_count = base_wave.get_warp_func_count(self._warp_type)
        max_count_reached = (warp_count >= base_wave.get_max_warp_func_count())
        self._adder.setVisible(not max_count_reached)


class WarpAdder(QPushButton, ProcessorUpdater):

    def __init__(self, warp_type, get_base_wave, get_update_signal_type):
        super().__init__()
        self._warp_type = warp_type
        self._get_base_wave = get_base_wave
        self._get_update_signal_type = get_update_signal_type

        self._add_text = { 'pre': 'Add prewarp', 'post': 'Add postwarp' }[warp_type]
        self.setText(self._add_text)

    def _on_setup(self):
        QObject.connect(self, SIGNAL('clicked()'), self._add_warp)

    def _add_warp(self):
        base_wave = self._get_base_wave()
        base_wave.add_warp_func(self._warp_type)
        self._updater.signal_update(self._get_update_signal_type())


class SmallButton(QPushButton):

    def __init__(self):
        super().__init__()
        self.setStyleSheet('padding: 0 -2px;')


class WarpEditor(QWidget, ProcessorUpdater):

    _ARG_SCALE = 1000

    def __init__(self, warp_type, index, get_base_wave, get_update_signal_type):
        super().__init__()
        self._warp_type = warp_type
        self._index = index
        self._get_base_wave = get_base_wave
        self._get_update_signal_type = get_update_signal_type

        self._down_button = SmallButton()
        self._up_button = SmallButton()
        self._func_selector = KqtComboBox()
        self._slider = QSlider(Qt.Horizontal)
        self._slider.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)
        self._value_display = QLabel()
        self._remove_button = SmallButton()

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

    def _on_setup(self):
        self.register_action('signal_au', self._update_all)
        self.register_action(self._get_update_signal_type(), self._update_all)

        icon_bank = self._ui_model.get_icon_bank()
        self._down_button.setIcon(QIcon(icon_bank.get_icon_path('arrow_down_small')))
        self._up_button.setIcon(QIcon(icon_bank.get_icon_path('arrow_up_small')))
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))

        base_wave = self._get_base_wave()

        func_names = base_wave.get_warp_func_names(self._warp_type)
        self._func_selector.set_items(name for name in func_names)

        self._update_all()

        QObject.connect(self._down_button, SIGNAL('clicked()'), self._moved_down)
        QObject.connect(self._up_button, SIGNAL('clicked()'), self._moved_up)
        QObject.connect(
                self._func_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._func_selected)
        QObject.connect(self._slider, SIGNAL('valueChanged(int)'), self._slider_adjusted)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._removed)

    def _update_all(self):
        base_wave = self._get_base_wave()

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
        base_wave = self._get_base_wave()
        base_wave.move_warp_func(self._warp_type, self._index, self._index + 1)
        self._updater.signal_update(self._get_update_signal_type())

    def _moved_up(self):
        base_wave = self._get_base_wave()
        base_wave.move_warp_func(self._warp_type, self._index, self._index - 1)
        self._updater.signal_update(self._get_update_signal_type())

    def _set_warp(self):
        base_wave = self._get_base_wave()
        name = str(self._func_selector.currentText())
        arg = self._slider.value() / float(self._ARG_SCALE)
        base_wave.set_warp_func(self._warp_type, self._index, name, arg)
        self._updater.signal_update(self._get_update_signal_type())

    def _func_selected(self, index):
        self._set_warp()

    def _slider_adjusted(self, int_val):
        self._set_warp()

    def _removed(self):
        base_wave = self._get_base_wave()
        base_wave.remove_warp_func(self._warp_type, self._index)
        self._updater.signal_update(self._get_update_signal_type())


