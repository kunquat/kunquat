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

from kunquat.tracker.ui.views.audiounit.aukeyboardmapper import AudioUnitKeyboardMapper
from .processorupdater import ProcessorUpdater


class ProcessorKeyboardMapper(AudioUnitKeyboardMapper, ProcessorUpdater):

    def process_typewriter_button_event(self, event):
        control_mgr = self._ui_model.get_control_manager()
        control_id = self.get_control_id()

        use_test_output = ((control_id != None) and
                self._is_processor_testable() and
                control_mgr.is_processor_testing_enabled(self._proc_id))

        if use_test_output:
            control_mgr.set_test_processor(control_id, self._proc_id)
        processed = super().process_typewriter_button_event(event)
        control_mgr.set_test_processor(control_id, None)

        return processed

    def _is_processor_testable(self):
        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        proc = au.get_processor(self._proc_id)

        return proc.get_existence() and (proc.get_signal_type() == 'voice')


