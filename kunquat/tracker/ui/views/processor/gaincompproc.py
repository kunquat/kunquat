# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.envelope import Envelope
from .procsimpleenv import ProcessorSimpleEnvelope
from .processorupdater import ProcessorUpdater


class GainCompProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Gain compression'

    def __init__(self):
        super().__init__()

        self._sym_toggle = QCheckBox('Adjust magnitude only')
        self._asym_mapping = AsymMappingEnv()
        self._sym_mapping = SymMappingEnv()

        self.add_to_updaters(self._asym_mapping, self._sym_mapping)

        self._mappings = QStackedLayout()
        self._mappings.addWidget(self._asym_mapping)
        self._mappings.addWidget(self._sym_mapping)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._sym_toggle)
        v.addLayout(self._mappings)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def _get_update_signal_type(self):
        return 'signal_gaincomp_mapping_{}'.format(self._proc_id)

    def _on_setup(self):
        self.register_action(self._get_update_signal_type(), self._update_sym)

        self._asym_mapping.configure_max_node_count()
        self._sym_mapping.configure_max_node_count()

        self._sym_toggle.stateChanged.connect(self._change_sym)

        self._update_sym()

    def _get_gc_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        gc_params = proc.get_type_params()
        return gc_params

    def _update_sym(self):
        gc_params = self._get_gc_params()

        self._mappings.setCurrentIndex(1 if gc_params.is_mapping_symmetric() else 0)

        old_block = self._sym_toggle.blockSignals(True)
        self._sym_toggle.setEnabled(gc_params.get_mapping_enabled())
        self._sym_toggle.setCheckState(
                Qt.Checked if gc_params.is_mapping_symmetric() else Qt.Unchecked)
        self._sym_toggle.blockSignals(old_block)

    def _change_sym(self, state):
        is_symmetric = (state == Qt.Checked)

        gc_params = self._get_gc_params()
        gc_params.set_mapping_symmetric(is_symmetric)

        self._updater.signal_update(self._get_update_signal_type())


class MappingEnv(ProcessorSimpleEnvelope):

    def __init__(self):
        self._mapping_env_view = None
        super().__init__()

    def configure_max_node_count(self):
        gc_params = self._get_gc_params()
        self._mapping_env_view.set_node_count_max(gc_params.get_max_node_count())

    def _get_update_signal_type(self):
        return 'signal_gaincomp_mapping_{}'.format(self._proc_id)

    def _get_title(self):
        return 'Signal mapping'

    def _get_gc_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        gc_params = proc.get_type_params()
        return gc_params


class AsymMappingEnv(MappingEnv):

    def __init__(self):
        super().__init__()

    def _make_envelope_widget(self):
        envelope = Envelope({ 'is_square_area': True })
        ev = envelope.get_envelope_view()
        ev.set_node_count_max(2)
        ev.set_y_range(-1, 1)
        ev.set_x_range(-1, 1)
        ev.set_first_lock(True, False)
        ev.set_last_lock(True, False)

        self._mapping_env_view = ev

        return envelope

    def _get_enabled(self):
        return self._get_gc_params().get_mapping_enabled()

    def _set_enabled(self, enabled):
        self._get_gc_params().set_mapping_enabled(enabled)

    def _get_envelope_data(self):
        return self._get_gc_params().get_mapping()

    def _set_envelope_data(self, envelope):
        gc_params = self._get_gc_params()
        if gc_params.is_mapping_symmetric():
            return

        self._get_gc_params().set_mapping(envelope)


class SymMappingEnv(MappingEnv):

    def __init__(self):
        super().__init__()

    def _make_envelope_widget(self):
        envelope = Envelope({ 'is_square_area': True })
        ev = envelope.get_envelope_view()
        ev.set_node_count_max(2)
        ev.set_y_range(0, 1)
        ev.set_x_range(0, 1)
        ev.set_first_lock(True, False)
        ev.set_last_lock(True, False)

        self._mapping_env_view = ev

        return envelope

    def _get_enabled(self):
        return self._get_gc_params().get_mapping_enabled()

    def _set_enabled(self, enabled):
        self._get_gc_params().set_mapping_enabled(enabled)

    def _get_envelope_data(self):
        return self._get_gc_params().get_mapping()

    def _set_envelope_data(self, envelope):
        gc_params = self._get_gc_params()
        if not gc_params.is_mapping_symmetric():
            return

        self._get_gc_params().set_mapping(envelope)


