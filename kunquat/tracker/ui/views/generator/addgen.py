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


class AddGen(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None

        self._base_waveform = WaveformEditor()

        base_layout = QVBoxLayout()
        base_layout.setSpacing(0)
        base_layout.addWidget(self._base_waveform)

        self._phase_mod_enabled_toggle = QCheckBox('Phase modulation')
        self._phase_mod_volume = ModVolume()
        self._phase_mod_env = ModEnv()

        self._phase_mod_container = QWidget()
        pmc_layout = QVBoxLayout()
        pmc_layout.setSpacing(0)
        pmc_layout.addWidget(self._phase_mod_volume)
        pmc_layout.addWidget(self._phase_mod_env)
        self._phase_mod_container.setLayout(pmc_layout)

        mod_layout = QVBoxLayout()
        mod_layout.setSpacing(0)
        mod_layout.addWidget(self._phase_mod_enabled_toggle)
        mod_layout.addWidget(self._phase_mod_container)

        h = QHBoxLayout()
        h.setSpacing(5)
        h.addLayout(base_layout)
        h.addLayout(mod_layout)
        self.setLayout(h)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id
        self._base_waveform.set_ins_id(ins_id)
        self._phase_mod_volume.set_ins_id(ins_id)
        self._phase_mod_env.set_ins_id(ins_id)

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id
        self._base_waveform.set_gen_id(gen_id)
        self._phase_mod_volume.set_gen_id(gen_id)
        self._phase_mod_env.set_gen_id(gen_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._base_waveform.set_ui_model(ui_model)
        self._phase_mod_volume.set_ui_model(ui_model)
        self._phase_mod_env.set_ui_model(ui_model)
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_gen()

        QObject.connect(
                self._phase_mod_enabled_toggle,
                SIGNAL('stateChanged(int)'),
                self._phase_mod_enabled_changed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._phase_mod_env.unregister_updaters()
        self._phase_mod_volume.unregister_updaters()
        self._base_waveform.unregister_updaters()

    def _get_update_signal_type(self):
        return ''.join(('signal_gen_add_', self._ins_id, self._gen_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_gen()

    def _get_add_params(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)
        add_params = generator.get_type_params()
        return add_params

    def _update_gen(self):
        add_params = self._get_add_params()
        phase_mod_enabled = add_params.get_phase_mod_enabled()

        old_block = self._phase_mod_enabled_toggle.blockSignals(True)
        self._phase_mod_enabled_toggle.setCheckState(
                Qt.Checked if phase_mod_enabled else Qt.Unchecked)
        self._phase_mod_enabled_toggle.blockSignals(old_block)

        self._phase_mod_container.setEnabled(phase_mod_enabled)

    def _phase_mod_enabled_changed(self, state):
        new_enabled = (state == Qt.Checked)
        add_params = self._get_add_params()
        add_params.set_phase_mod_enabled(new_enabled)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class WaveformEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ins_id = None
        self._gen_id = None
        self._ui_model = None
        self._updater = None

        self._prewarp_list = WarpList('Add prewarp')
        self._base_func_selector = QComboBox()
        self._postwarp_list = WarpList('Add postwarp')
        self._waveform = Waveform()

        v = QVBoxLayout()
        v.setSpacing(0)
        v.addWidget(self._prewarp_list)
        v.addWidget(self._base_func_selector)
        v.addWidget(self._postwarp_list)
        v.addWidget(self._waveform)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

        add_params = self._get_add_params()

        self._prewarp_list.set_func_names(add_params.get_prewarp_func_names())
        self._postwarp_list.set_func_names(add_params.get_postwarp_func_names())

        icon_bank = self._ui_model.get_icon_bank()
        self._prewarp_list.set_icon_bank(icon_bank)
        self._postwarp_list.set_icon_bank(icon_bank)

        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_all()

        QObject.connect(
                self._base_func_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._base_func_selected)

        QObject.connect(self._prewarp_list, SIGNAL('warpAdded()'), self._prewarp_added)
        QObject.connect(
                self._prewarp_list, SIGNAL('warpChanged(int)'), self._prewarp_changed)
        QObject.connect(
                self._prewarp_list, SIGNAL('warpMoved(int, int)'), self._prewarp_moved)
        QObject.connect(
                self._prewarp_list, SIGNAL('warpRemoved(int)'), self._prewarp_removed)

        QObject.connect(self._postwarp_list, SIGNAL('warpAdded()'), self._postwarp_added)
        QObject.connect(
                self._postwarp_list, SIGNAL('warpChanged(int)'), self._postwarp_changed)
        QObject.connect(
                self._postwarp_list, SIGNAL('warpMoved(int, int)'), self._postwarp_moved)
        QObject.connect(
                self._postwarp_list, SIGNAL('warpRemoved(int)'), self._postwarp_removed)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _get_update_signal_type(self):
        return ''.join(('signal_gen_add_base_', self._ins_id, self._gen_id))

    def _perform_updates(self, signals):
        update_signals = set(['signal_instrument', self._get_update_signal_type()])
        if not signals.isdisjoint(update_signals):
            self._update_all()

    def _get_add_params(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)
        add_params = generator.get_type_params()
        return add_params

    def _update_all(self):
        add_params = self._get_add_params()

        selected_base_func = add_params.get_base_waveform_func()
        enable_warps = (selected_base_func != None)

        self._prewarp_list.setEnabled(enable_warps)
        self._prewarp_list.set_warp_count(add_params.get_prewarp_func_count())
        for i in xrange(add_params.get_prewarp_func_count()):
            name, arg = add_params.get_prewarp_func(i)
            self._prewarp_list.set_warp(i, name, arg)

        old_block = self._base_func_selector.blockSignals(True)
        self._base_func_selector.clear()
        func_names = add_params.get_base_waveform_func_names()
        for i, name in enumerate(func_names):
            self._base_func_selector.addItem(name)
            if name == selected_base_func:
                self._base_func_selector.setCurrentIndex(i)
        if not selected_base_func:
            self._base_func_selector.addItem('Custom')
            self._base_func_selector.setCurrentIndex(len(func_names))
        self._base_func_selector.blockSignals(old_block)

        self._postwarp_list.setEnabled(enable_warps)
        self._postwarp_list.set_warp_count(add_params.get_postwarp_func_count())
        for i in xrange(add_params.get_postwarp_func_count()):
            name, arg = add_params.get_postwarp_func(i)
            self._postwarp_list.set_warp(i, name, arg)

        self._waveform.set_waveform(add_params.get_base_waveform())

    def _prewarp_added(self):
        add_params = self._get_add_params()
        add_params.add_prewarp_func()
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _prewarp_changed(self, index):
        add_params = self._get_add_params()
        name, arg = self._prewarp_list.get_warp(index)
        add_params.set_prewarp_func(index, name, arg)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _prewarp_moved(self, from_index, to_index):
        add_params = self._get_add_params()
        add_params.move_prewarp_func(from_index, to_index)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _prewarp_removed(self, index):
        add_params = self._get_add_params()
        add_params.remove_prewarp_func(index)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _base_func_selected(self, index):
        add_params = self._get_add_params()
        func_names = add_params.get_base_waveform_func_names()
        add_params.set_base_waveform_func(func_names[index])
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _postwarp_added(self):
        add_params = self._get_add_params()
        add_params.add_postwarp_func()
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _postwarp_changed(self, index):
        add_params = self._get_add_params()
        name, arg = self._postwarp_list.get_warp(index)
        add_params.set_postwarp_func(index, name, arg)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _postwarp_moved(self, from_index, to_index):
        add_params = self._get_add_params()
        add_params.move_postwarp_func(from_index, to_index)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _postwarp_removed(self, index):
        add_params = self._get_add_params()
        add_params.remove_postwarp_func(index)
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

    warpAdded   = pyqtSignal(name='warpAdded')
    warpChanged = pyqtSignal(int, name='warpChanged')
    warpMoved   = pyqtSignal(int, int, name='warpMoved')
    warpRemoved = pyqtSignal(int, name='warpRemoved')

    def __init__(self, add_text):
        QAbstractScrollArea.__init__(self)
        self._func_names = None
        self._icon_bank = None
        self._add_text = add_text

        self._init_container()

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)

    def _init_container(self):
        self.setWidget(WarpListContainer())
        add_button = QPushButton(self._add_text)
        QObject.connect(add_button, SIGNAL('clicked()'), self._warp_added)
        self.widget().layout().addWidget(add_button)

    def set_func_names(self, func_names):
        self._func_names = func_names

    def set_icon_bank(self, icon_bank):
        self._icon_bank = icon_bank

    def set_warp_count(self, count):
        layout = self.widget().layout()

        if count < layout.count() - 1:
            # Create contents from scratch because Qt sucks
            self._init_container()
            layout = self.widget().layout()

        # Create new items
        for i in xrange(layout.count() - 1, count):
            editor = WarpEditor(i, self._icon_bank)
            editor.set_func_names(self._func_names)
            QObject.connect(editor, SIGNAL('warpChanged(int)'), self._warp_changed)
            QObject.connect(editor, SIGNAL('warpMoved(int, int)'), self._warp_moved)
            QObject.connect(editor, SIGNAL('warpRemoved(int)'), self._warp_removed)
            layout.insertWidget(i, editor)

        # Update arrows
        for i in xrange(layout.count() - 1):
            editor = layout.itemAt(i).widget()
            down_enabled = i < layout.count() - 2
            up_enabled = i > 0
            editor.set_arrow_buttons_enabled(down_enabled, up_enabled)

        self._do_width_hack()

    def set_warp(self, index, name, arg):
        editor = self.widget().layout().itemAt(index).widget()
        editor.set_warp(name, arg)

    def get_warp(self, index):
        editor = self.widget().layout().itemAt(index).widget()
        return editor.get_warp()

    def _warp_added(self):
        QObject.emit(self, SIGNAL('warpAdded()'))

    def _warp_changed(self, index):
        QObject.emit(self, SIGNAL('warpChanged(int)'), index)

    def _warp_moved(self, from_index, to_index):
        QObject.emit(self, SIGNAL('warpMoved(int, int)'), from_index, to_index)

    def _warp_removed(self, index):
        QObject.emit(self, SIGNAL('warpRemoved(int)'), index)

    def _do_width_hack(self):
        self.widget().setFixedWidth(
                self.width() - self.verticalScrollBar().width() - 5)

    def resizeEvent(self, event):
        self._do_width_hack()


class WarpEditorButton(QPushButton):

    def __init__(self, icon):
        QPushButton.__init__(self)
        self.setStyleSheet('padding: 0 -2px;')
        self.setIcon(QIcon(icon))


class WarpEditor(QWidget):

    warpChanged = pyqtSignal(int, name='warpChanged')
    warpMoved   = pyqtSignal(int, int, name='warpMoved')
    warpRemoved = pyqtSignal(int, name='warpRemoved')

    _ARG_SCALE = 1000

    def __init__(self, index, icon_bank):
        QWidget.__init__(self)
        self._index = index

        self._down_button = WarpEditorButton(icon_bank.get_icon_path('arrow_down_small'))
        self._up_button = WarpEditorButton(icon_bank.get_icon_path('arrow_up_small'))
        self._func_selector = QComboBox()
        self._slider = QSlider(Qt.Horizontal)
        self._slider.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)
        self._value_display = QLabel()
        self._remove_button = WarpEditorButton(icon_bank.get_icon_path('delete_small'))

        self._slider.setRange(-self._ARG_SCALE, self._ARG_SCALE)

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

        QObject.connect(self._down_button, SIGNAL('clicked()'), self._moved_down)
        QObject.connect(self._up_button, SIGNAL('clicked()'), self._moved_up)
        QObject.connect(
                self._func_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._func_selected)
        QObject.connect(self._slider, SIGNAL('valueChanged(int)'), self._slider_adjusted)
        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._removed)

    def set_func_names(self, func_names):
        old_block = self._func_selector.blockSignals(True)
        self._func_selector.clear()
        for name in func_names:
            self._func_selector.addItem(name)
        self._func_selector.blockSignals(old_block)

    def set_arrow_buttons_enabled(self, down_enabled, up_enabled):
        self._down_button.setEnabled(down_enabled)
        self._up_button.setEnabled(up_enabled)

    def set_warp(self, new_name, new_arg):
        old_block = self._func_selector.blockSignals(True)
        for i in xrange(self._func_selector.count()):
            if self._func_selector.itemText(i) == new_name:
                self._func_selector.setCurrentIndex(i)
                break
        self._func_selector.blockSignals(old_block)

        old_block = self._slider.blockSignals(True)
        int_val = int(round(new_arg * self._ARG_SCALE))
        self._slider.setValue(int_val)
        self._slider.blockSignals(old_block)

        self._value_display.setText(str(float(new_arg)))

    def get_warp(self):
        name = str(self._func_selector.currentText())
        arg = self._slider.value() / float(self._ARG_SCALE)
        return name, arg

    def _moved_down(self):
        QObject.emit(self, SIGNAL('warpMoved(int, int)'), self._index, self._index + 1)

    def _moved_up(self):
        QObject.emit(self, SIGNAL('warpMoved(int, int)'), self._index, self._index - 1)

    def _func_selected(self, index):
        QObject.emit(self, SIGNAL('warpChanged(int)'), self._index)

    def _slider_adjusted(self, int_val):
        QObject.emit(self, SIGNAL('warpChanged(int)'), self._index)

    def _removed(self):
        QObject.emit(self, SIGNAL('warpRemoved(int)'), self._index)


class ModVolume(GenNumSlider):

    def __init__(self):
        GenNumSlider.__init__(self, 2, -64.0, 24.0, title='Mod volume')
        self.set_number(0)

    def _get_add_params(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)
        add_params = generator.get_type_params()
        return add_params

    def _update_value(self):
        add_params = self._get_add_params()
        self.set_number(add_params.get_mod_volume())

    def _value_changed(self, mod_volume):
        add_params = self._get_add_params()
        add_params.set_mod_volume(mod_volume)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return ''.join(('signal_add_mod_volume_', self._ins_id, self._gen_id))


class ModEnv(TimeEnvelope):

    def __init__(self):
        TimeEnvelope.__init__(self)
        self._gen_id = None

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
        return self._get_add_params().get_mod_envelope_enabled()

    def _set_enabled(self, enabled):
        self._get_add_params().set_mod_envelope_enabled(enabled)

    def _get_scale_amount(self):
        return self._get_add_params().get_mod_envelope_scale_amount()

    def _set_scale_amount(self, value):
        self._get_add_params().set_mod_envelope_scale_amount(value)

    def _get_scale_center(self):
        return self._get_add_params().get_mod_envelope_scale_center()

    def _set_scale_center(self, value):
        self._get_add_params().set_mod_envelope_scale_center(value)

    def _get_envelope_data(self):
        return self._get_add_params().get_mod_envelope()

    def _set_envelope_data(self, envelope):
        self._get_add_params().set_mod_envelope(envelope)

    def _get_add_params(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        generator = instrument.get_generator(self._gen_id)
        add_params = generator.get_type_params()
        return add_params


