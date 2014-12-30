# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
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

        self._prewarp_sliders = []
        self._base_func_selector = QComboBox()
        self._postwarp_sliders = []
        self._waveform = Waveform()

        v = QVBoxLayout()
        for widget in self._prewarp_sliders:
            v.addWidget(widget)
        v.addWidget(self._base_func_selector)
        for widget in self._postwarp_sliders:
            v.addWidget(widget)
        v.addWidget(self._waveform)
        self.setLayout(v)

    def set_ins_id(self, ins_id):
        self._ins_id = ins_id

    def set_gen_id(self, gen_id):
        self._gen_id = gen_id

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._update_all()

        QObject.connect(
                self._base_func_selector,
                SIGNAL('currentIndexChanged(int)'),
                self._base_func_selected)

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
        old_block = self._base_func_selector.blockSignals(True)
        self._base_func_selector.clear()
        for i, name in enumerate(add_params.get_base_waveform_func_names()):
            self._base_func_selector.addItem(name)
            if name == selected_base_func:
                self._base_func_selector.setCurrentIndex(i)
        self._base_func_selector.blockSignals(old_block)

        self._waveform.set_waveform(add_params.get_base_waveform())

    def _base_func_selected(self, index):
        add_params = self._get_add_params()
        func_names = add_params.get_base_waveform_func_names()
        add_params.set_base_waveform_func(func_names[index])
        self._updater.signal_update(set([self._get_update_signal_type()]))


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


