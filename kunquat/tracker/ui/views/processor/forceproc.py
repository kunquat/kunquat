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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from procnumslider import ProcNumSlider
from kunquat.tracker.ui.views.envelope import Envelope
from kunquat.tracker.ui.views.audio_unit.time_env import TimeEnvelope


class ForceProc(QWidget):

    @staticmethod
    def get_name():
        return u'Force'

    def __init__(self):
        QWidget.__init__(self)

        self._global_force = GlobalForceSlider()
        self._force_variation = ForceVarSlider()
        self._force_envelope = ForceEnvelope()

        sliders = QGridLayout()
        sliders.addWidget(QLabel('Global force:'), 0, 0)
        sliders.addWidget(self._global_force, 0, 1)
        sliders.addWidget(QLabel('Force variation:'), 1, 0)
        sliders.addWidget(self._force_variation, 1, 1)

        v = QVBoxLayout()
        v.addLayout(sliders)
        v.addWidget(self._force_envelope)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._global_force.set_au_id(au_id)
        self._force_variation.set_au_id(au_id)
        self._force_envelope.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._global_force.set_proc_id(proc_id)
        self._force_variation.set_proc_id(proc_id)
        self._force_envelope.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._global_force.set_ui_model(ui_model)
        self._force_variation.set_ui_model(ui_model)
        self._force_envelope.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._force_envelope.unregister_updaters()
        self._force_variation.unregister_updaters()
        self._global_force.unregister_updaters()


class ForceNumSlider(ProcNumSlider):

    def __init__(self, decimals, min_value, max_value):
        ProcNumSlider.__init__(self, decimals, min_value, max_value, '')

    def _get_force_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        force_params = proc.get_type_params()
        return force_params


class GlobalForceSlider(ForceNumSlider):

    def __init__(self):
        ForceNumSlider.__init__(self, 1, -64.0, 18.0)

    def _get_update_signal_type(self):
        return '_'.join(('signal_force_global_force', self._proc_id))

    def _update_value(self):
        force_params = self._get_force_params()
        self.set_number(force_params.get_global_force())

    def _value_changed(self, value):
        force_params = self._get_force_params()
        force_params.set_global_force(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class ForceVarSlider(ForceNumSlider):

    def __init__(self):
        ForceNumSlider.__init__(self, 1, 0.0, 32.0)

    def _get_update_signal_type(self):
        return '_'.join(('signal_force_force_variation', self._proc_id))

    def _update_value(self):
        force_params = self._get_force_params()
        self.set_number(force_params.get_force_variation())

    def _value_changed(self, value):
        force_params = self._get_force_params()
        force_params.set_force_variation(value)
        self._updater.signal_update(set([self._get_update_signal_type()]))


class ForceEnvelope(TimeEnvelope):

    def __init__(self):
        TimeEnvelope.__init__(self)
        self._proc_id = None

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def _get_title(self):
        return u'Force envelope'

    def _allow_loop(self):
        return True

    def _make_envelope_widget(self):
        envelope = Envelope()
        envelope.set_node_count_max(32)
        envelope.set_y_range(0, 1)
        envelope.set_x_range(0, 4)
        envelope.set_first_lock(True, False)
        envelope.set_x_range_adjust(False, True)
        return envelope

    def _get_update_signal_type(self):
        return '_'.join(('signal_force_envelope', self._proc_id))

    def _get_force_params(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)
        force_params = proc.get_type_params()
        return force_params

    def _get_enabled(self):
        return self._get_force_params().get_envelope_enabled()

    def _set_enabled(self, enabled):
        self._get_force_params().set_envelope_enabled(enabled)

    def _get_loop_enabled(self):
        return self._get_force_params().get_envelope_loop_enabled()

    def _set_loop_enabled(self, enabled):
        self._get_force_params().set_envelope_loop_enabled(enabled)

    def _get_scale_amount(self):
        return self._get_force_params().get_envelope_scale_amount()

    def _set_scale_amount(self, value):
        self._get_force_params().set_envelope_scale_amount(value)

    def _get_scale_center(self):
        return self._get_force_params().get_envelope_scale_center()

    def _set_scale_center(self, value):
        self._get_force_params().set_envelope_scale_center(value)

    def _get_envelope_data(self):
        return self._get_force_params().get_envelope()

    def _set_envelope_data(self, envelope):
        self._get_force_params().set_envelope(envelope)


