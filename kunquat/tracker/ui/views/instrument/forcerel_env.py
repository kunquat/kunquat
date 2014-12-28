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

from kunquat.tracker.ui.views.envelope import Envelope
from time_env import TimeEnvelope


class ForceReleaseEnvelope(TimeEnvelope):

    def __init__(self):
        TimeEnvelope.__init__(self)

    def _get_title(self):
        return 'Force release envelope'

    def _allow_loop(self):
        return False

    def _make_envelope_widget(self):
        envelope = Envelope()
        envelope.set_node_count_max(32)
        envelope.set_y_range(0, 1)
        envelope.set_x_range(0, 4)
        envelope.set_first_lock(True, False)
        envelope.set_last_lock(False, True)
        envelope.set_x_range_adjust(False, True)
        return envelope

    def _get_update_signal_type(self):
        return ''.join(('signal_forcerel_env_', self._ins_id))

    def _get_enabled(self):
        return self._get_instrument().get_force_release_envelope_enabled()

    def _set_enabled(self, enabled):
        self._get_instrument().set_force_release_envelope_enabled(enabled)

    def _get_scale_amount(self):
        return self._get_instrument().get_force_release_envelope_scale_amount()

    def _set_scale_amount(self, value):
        self._get_instrument().set_force_release_envelope_scale_amount(value)

    def _get_scale_center(self):
        return self._get_instrument().get_force_release_envelope_scale_center()

    def _set_scale_center(self, value):
        self._get_instrument().set_force_release_envelope_scale_center(value)

    def _get_envelope_data(self):
        return self._get_instrument().get_force_release_envelope()

    def _set_envelope_data(self, envelope):
        self._get_instrument().set_force_release_envelope(envelope)

    def _get_instrument(self):
        module = self._ui_model.get_module()
        instrument = module.get_instrument(self._ins_id)
        return instrument


