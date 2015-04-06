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

from kunquat.tracker.ui.views.envelope import Envelope
from time_env import TimeEnvelope


class FilterEditor(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._au_id = None
        self._ui_model = None

        self._filter_env = FilterEnvelope()
        self._filter_rel_env = FilterReleaseEnvelope()

        v = QVBoxLayout()
        v.setMargin(8)
        v.setSpacing(8)
        v.addWidget(self._filter_env)
        v.addWidget(self._filter_rel_env)
        self.setLayout(v)

    def set_au_id(self, au_id):
        self._au_id = au_id
        self._filter_env.set_au_id(au_id)
        self._filter_rel_env.set_au_id(au_id)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._filter_env.set_ui_model(ui_model)
        self._filter_rel_env.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._filter_rel_env.unregister_updaters()
        self._filter_env.unregister_updaters()


class FilterEnvelope(TimeEnvelope):

    def __init__(self):
        TimeEnvelope.__init__(self)

    def _get_title(self):
        return 'Filter envelope'

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
        return '_'.join(('signal_filter_env', self._au_id))

    def _get_enabled(self):
        return self._get_audio_unit().get_filter_envelope_enabled()

    def _set_enabled(self, enabled):
        self._get_audio_unit().set_filter_envelope_enabled(enabled)

    def _get_loop_enabled(self):
        return self._get_audio_unit().get_filter_envelope_loop_enabled()

    def _set_loop_enabled(self, enabled):
        self._get_audio_unit().set_filter_envelope_loop_enabled(enabled)

    def _get_scale_amount(self):
        return self._get_audio_unit().get_filter_envelope_scale_amount()

    def _set_scale_amount(self, value):
        self._get_audio_unit().set_filter_envelope_scale_amount(value)

    def _get_scale_center(self):
        return self._get_audio_unit().get_filter_envelope_scale_center()

    def _set_scale_center(self, value):
        self._get_audio_unit().set_filter_envelope_scale_center(value)

    def _get_envelope_data(self):
        return self._get_audio_unit().get_filter_envelope()

    def _set_envelope_data(self, envelope):
        self._get_audio_unit().set_filter_envelope(envelope)

    def _get_audio_unit(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au


class FilterReleaseEnvelope(TimeEnvelope):

    def __init__(self):
        TimeEnvelope.__init__(self)

    def _get_title(self):
        return 'Filter release envelope'

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
        return '_'.join(('signal_filter_rel_env', self._au_id))

    def _get_enabled(self):
        return self._get_audio_unit().get_filter_release_envelope_enabled()

    def _set_enabled(self, enabled):
        self._get_audio_unit().set_filter_release_envelope_enabled(enabled)

    def _get_scale_amount(self):
        return self._get_audio_unit().get_filter_release_envelope_scale_amount()

    def _set_scale_amount(self, value):
        self._get_audio_unit().set_filter_release_envelope_scale_amount(value)

    def _get_scale_center(self):
        return self._get_audio_unit().get_filter_release_envelope_scale_center()

    def _set_scale_center(self, value):
        self._get_audio_unit().set_filter_release_envelope_scale_center(value)

    def _get_envelope_data(self):
        return self._get_audio_unit().get_filter_release_envelope()

    def _set_envelope_data(self, envelope):
        self._get_audio_unit().set_filter_release_envelope(envelope)

    def _get_audio_unit(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        return au


