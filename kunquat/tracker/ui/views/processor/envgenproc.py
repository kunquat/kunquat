# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
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
from kunquat.tracker.ui.views.headerline import HeaderLine
from kunquat.tracker.ui.views.audio_unit.simple_env import SimpleEnvelope
from kunquat.tracker.ui.views.audio_unit.time_env import TimeEnvelope


def get_egen_params(obj):
    module = obj._ui_model.get_module()
    au = module.get_audio_unit(obj._au_id)
    proc = au.get_processor(obj._proc_id)
    egen_params = proc.get_type_params()
    return egen_params


class EnvgenProc(QWidget):

    @staticmethod
    def get_name():
        return u'Envelope generation'

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._proc_id = None
        self._ui_model = None

        # TODO: range edit

        self._scale = ScaleSlider()
        self._time_env = EgenTimeEnv()
        self._force_env = ForceEnv()

        v = QVBoxLayout()
        v.setSpacing(10)
        v.addWidget(self._scale)
        v.addWidget(self._time_env)
        v.addWidget(self._force_env)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._scale.set_au_id(au_id)
        self._time_env.set_au_id(au_id)
        self._force_env.set_au_id(au_id)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id
        self._scale.set_proc_id(proc_id)
        self._time_env.set_proc_id(proc_id)
        self._force_env.set_proc_id(proc_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._scale.set_ui_model(ui_model)
        self._time_env.set_ui_model(ui_model)
        self._force_env.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._force_env.unregister_updaters()
        self._time_env.unregister_updaters()
        self._scale.unregister_updaters()


class ScaleSlider(ProcNumSlider):

    def __init__(self):
        ProcNumSlider.__init__(self, 2, -64.0, 24.0, title='Scale')
        self.set_number(0)

    def _update_value(self):
        egen_params = get_egen_params(self)
        self.set_number(egen_params.get_scale())

    def _value_changed(self, scale):
        egen_params = get_egen_params(self)
        egen_params.set_scale(scale)
        self._updater.signal_update(set([self._get_update_signal_type()]))

    def _get_update_signal_type(self):
        return ''.join(('signal_egen_scale_', self._au_id, self._proc_id))


class EgenTimeEnv(TimeEnvelope):

    def __init__(self):
        TimeEnvelope.__init__(self)
        self._proc_id = None

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def _get_title(self):
        return 'Envelope'

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
        return ''.join(('signal_egen_time_env_', self._au_id, self._proc_id))

    def _get_enabled(self):
        return get_egen_params(self).get_time_env_enabled()

    def _set_enabled(self, enabled):
        get_egen_params(self).set_time_env_enabled(enabled)

    def _get_loop_enabled(self):
        return get_egen_params(self).get_time_env_loop_enabled()

    def _set_loop_enabled(self, enabled):
        get_egen_params(self).set_time_env_loop_enabled(enabled)

    def _get_scale_amount(self):
        return get_egen_params(self).get_time_env_scale_amount()

    def _set_scale_amount(self, value):
        get_egen_params(self).set_time_env_scale_amount(value)

    def _get_scale_center(self):
        return get_egen_params(self).get_time_env_scale_center()

    def _set_scale_center(self, value):
        get_egen_params(self).set_time_env_scale_center(value)

    def _get_envelope_data(self):
        return get_egen_params(self).get_time_env()

    def _set_envelope_data(self, envelope):
        get_egen_params(self).set_time_env(envelope)


class ForceEnv(SimpleEnvelope):

    def __init__(self):
        SimpleEnvelope.__init__(self)
        self._proc_id = None

    def set_proc_id(self, proc_id):
        self._proc_id = proc_id

    def _get_update_signal_type(self):
        return ''.join(('signal_add_force_mod_volume_', self._au_id, self._proc_id))

    def _get_title(self):
        return 'Force envelope'

    def _make_envelope_widget(self):
        envelope = Envelope({ 'is_square_area': True })
        envelope.set_node_count_max(32)
        envelope.set_y_range(0, 1)
        envelope.set_x_range(0, 1)
        envelope.set_first_lock(True, False)
        envelope.set_last_lock(True, False)
        return envelope

    def _get_enabled(self):
        egen_params = get_egen_params(self)
        return egen_params.get_force_env_enabled()

    def _set_enabled(self, enabled):
        egen_params = get_egen_params(self)
        egen_params.set_force_env_enabled(enabled)

    def _get_envelope_data(self):
        egen_params = get_egen_params(self)
        return egen_params.get_force_env()

    def _set_envelope_data(self, envelope):
        egen_params = get_egen_params(self)
        egen_params.set_force_env(envelope)


