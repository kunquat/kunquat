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


class PadsynthProc(QWidget):

    @staticmethod
    def get_name():
        return 'PADsynth'

    def __init__(self):
        super().__init__()

        self._apply_button = ApplyButton()
        self._harmonics = Harmonics()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(4)
        v.addWidget(self._apply_button)
        v.addWidget(self._harmonics)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._apply_button.set_au_id(au_id)
        self._harmonics.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._apply_button.set_proc_id(proc_id)
        self._harmonics.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._apply_button.set_ui_model(ui_model)
        self._harmonics.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._harmonics.unregister_updaters()
        self._apply_button.unregister_updaters()


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


class HarmonicsList(EditorList):

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
        adder = HarmonicAdder()
        adder.set_au_id(self._au_id)
        adder.set_proc_id(self._proc_id)
        adder.set_ui_model(self._ui_model)
        return adder

    def _get_updated_editor_count(self):
        params = utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)
        count = params.get_harmonics().get_count()
        return count

    def _make_editor_widget(self, index):
        editor = HarmonicEditor(index)
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


class HarmonicAdder(QPushButton):

    def __init__(self):
        super().__init__()
        self._au_id = None
        self._proc_id = None
        self._ui_model = None
        self._updater = None

        self.setText('Add harmonic')

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

        harmonics = params.get_harmonics()
        harmonics.append_harmonic()
        self._updater.signal_update(set([self._get_update_signal_type()]))


class HarmonicEditor(QWidget):

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

        self._bandwidth = BandwidthEditor(index)

        self._remove_button = QPushButton()
        self._remove_button.setStyleSheet('padding: 0 -2px;')
        self._remove_button.setEnabled(self._index != 0)

        h = QHBoxLayout()
        h.setContentsMargins(4, 0, 4, 0)
        h.setSpacing(2)
        h.addWidget(QLabel('Pitch factor:'))
        h.addWidget(self._pitch_factor)
        h.addWidget(self._amplitude)
        h.addWidget(self._bandwidth)
        h.addWidget(self._remove_button)
        self.setLayout(h)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._amplitude.set_au_id(au_id)
        self._bandwidth.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._amplitude.set_proc_id(proc_id)
        self._bandwidth.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()

        self._amplitude.set_ui_model(ui_model)
        self._bandwidth.set_ui_model(ui_model)

        icon_bank = self._ui_model.get_icon_bank()
        self._remove_button.setIcon(QIcon(icon_bank.get_icon_path('delete_small')))

        QObject.connect(
                self._pitch_factor,
                SIGNAL('valueChanged(double)'),
                self._change_pitch_factor)

        QObject.connect(self._remove_button, SIGNAL('clicked()'), self._remove_harmonic)

        self.update_index(self._index)

    def unregister_updaters(self):
        self._bandwidth.unregister_updaters()
        self._amplitude.unregister_updaters()

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def update_index(self, index):
        self._index = index

        harmonics = self._get_params().get_harmonics()
        if self._index >= harmonics.get_count():
            return

        harmonic = harmonics.get_harmonic(self._index)

        old_block = self._pitch_factor.blockSignals(True)
        new_pitch_factor = harmonic.get_freq_mul()
        if new_pitch_factor != self._pitch_factor.value():
            self._pitch_factor.setValue(new_pitch_factor)
        self._pitch_factor.blockSignals(old_block)

        self._remove_button.setEnabled(harmonics.get_count() > 1)

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _change_pitch_factor(self, value):
        harmonics = self._get_params().get_harmonics()
        if self._index >= harmonics.get_count():
            return

        harmonic = harmonics.get_harmonic(self._index)
        harmonic.set_freq_mul(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _remove_harmonic(self):
        harmonics = self._get_params().get_harmonics()
        if self._index >= harmonics.get_count():
            return

        harmonics.remove_harmonic(self._index)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class AmplitudeEditor(ProcNumSlider):

    def __init__(self, index):
        super().__init__(3, 0.0, 1.0, title='Amplitude:')
        self._index = index

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_value(self):
        harmonics = self._get_params().get_harmonics()
        if self._index >= harmonics.get_count():
            return

        harmonic = harmonics.get_harmonic(self._index)
        self.set_number(harmonic.get_amplitude())

    def _value_changed(self, amplitude):
        harmonics = self._get_params().get_harmonics()
        if self._index >= harmonics.get_count():
            return

        harmonic = harmonics.get_harmonic(self._index)
        harmonic.set_amplitude(amplitude)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class BandwidthEditor(ProcNumSlider):

    def __init__(self, index):
        super().__init__(1, 0.1, 1200.0, title='Bandwidth:')
        self._index = index

    def _get_update_signal_type(self):
        return 'signal_padsynth_{}'.format(self._proc_id)

    def _get_params(self):
        return utils.get_proc_params(self._ui_model, self._au_id, self._proc_id)

    def _update_value(self):
        harmonics = self._get_params().get_harmonics()
        if self._index >= harmonics.get_count():
            return

        harmonic = harmonics.get_harmonic(self._index)
        self.set_number(harmonic.get_bandwidth())

    def _value_changed(self, bandwidth):
        harmonics = self._get_params().get_harmonics()
        if self._index >= harmonics.get_count():
            return

        harmonic = harmonics.get_harmonic(self._index)
        harmonic.set_bandwidth(bandwidth)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class Harmonics(QWidget):

    def __init__(self):
        super().__init__()

        self._editor = HarmonicsList()

        v = QVBoxLayout()
        v.addWidget(HeaderLine('Harmonics'))
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


