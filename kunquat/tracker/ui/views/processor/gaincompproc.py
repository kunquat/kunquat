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

from PySide.QtCore import *
from PySide.QtGui import *

from kunquat.tracker.ui.views.envelope import Envelope
from .procsimpleenv import ProcessorSimpleEnvelope
from .processorupdater import ProcessorUpdater


class GainCompProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Gain compression'

    def __init__(self):
        super().__init__()

        self._mapping = MappingEnv()

        self.add_updating_child(self._mapping)

        v = QVBoxLayout()
        v.setSpacing(10)
        v.addWidget(self._mapping)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)


class MappingEnv(ProcessorSimpleEnvelope):

    def __init__(self):
        super().__init__()

    def _get_update_signal_type(self):
        return ''.join(('signal_gaincomp_mapping_', self._proc_id))

    def _get_title(self):
        return 'Signal mapping'

    def _make_envelope_widget(self):
        envelope = Envelope({ 'is_square_area': True })
        envelope.set_node_count_max(32)
        envelope.set_y_range(0, 1)
        envelope.set_x_range(0, 1)
        envelope.set_first_lock(True, False)
        envelope.set_last_lock(True, False)
        return envelope

    def _get_gc_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        gc_params = proc.get_type_params()
        return gc_params

    def _get_enabled(self):
        return self._get_gc_params().get_mapping_enabled()

    def _set_enabled(self, enabled):
        self._get_gc_params().set_mapping_enabled(enabled)

    def _get_envelope_data(self):
        return self._get_gc_params().get_mapping()

    def _set_envelope_data(self, envelope):
        self._get_gc_params().set_mapping(envelope)


