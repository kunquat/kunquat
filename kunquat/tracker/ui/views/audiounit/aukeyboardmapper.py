# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.views.keyboardmapper import KeyboardMapper
from .updatingauview import UpdatingAUView


class AudioUnitKeyboardMapper(KeyboardMapper, UpdatingAUView):

    def get_control_id(self):
        module = self._ui_model.get_module()
        return module.get_control_id_by_au_id(self._au_id)

    def process_typewriter_button_event(self, event):
        control_id = self.get_control_id()
        if not control_id:
            # TODO: Check if we are inside another audio unit
            return super().process_typewriter_button_event(event)

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        control_manager = self._ui_model.get_control_manager()

        control_manager.set_control_id_override(control_id)
        au.set_test_params_enabled(True)
        processed = super().process_typewriter_button_event(event)
        control_manager.set_control_id_override(None)
        au.set_test_params_enabled(False)

        return processed


