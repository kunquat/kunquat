# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2018
#          Toni Ruottu, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.config import get_config
from kunquat.tracker.ui.identifiers import *


class VisibilityManager():

    def __init__(self):
        self._controller = None
        self._session = None
        self._updater = None
        self._is_ui_visible = True
        self._is_closing = False

    def set_controller(self, controller):
        self._controller = controller
        self._session = controller.get_session()
        self._updater = controller.get_updater()

    def run_hidden(self):
        self._is_ui_visible = False

    def is_show_allowed(self):
        return self._is_ui_visible and not self._is_closing

    def hide_all(self):
        self._is_closing = True
        self._session.hide_all()

    def show_main(self):
        self._show_window(UI_MAIN)

    def hide_main(self):
        self._hide_window(UI_MAIN)
        self.hide_all()

    def hide_main_after_saving(self):
        self._session.hide_ui(UI_MAIN)
        self.hide_all()
        self._updater.signal_update_deferred('signal_visibility')

    def set_input_control_view(self, mode):
        config = get_config()
        config.set_value('input_control_view', mode)

    def get_input_control_view(self):
        config = get_config()
        return config.get_value('input_control_view')

    def _show_window(self, ui):
        if self._is_closing:
            return
        if ui in self.get_visible():
            self._session.signal_ui(ui)
        self._session.show_ui(ui)

        self._updater.signal_update('signal_visibility')

    def _hide_window(self, ui):
        self._session.hide_ui(ui)
        self._updater.signal_update('signal_visibility')

    def show_about(self):
        self._show_window(UI_ABOUT)

    def hide_about(self):
        self._hide_window(UI_ABOUT)

    def show_event_log(self):
        self._show_window(UI_EVENT_LOG)

    def hide_event_log(self):
        self._hide_window(UI_EVENT_LOG)

    def show_connections(self):
        self._show_window(UI_CONNECTIONS)

    def hide_connections(self):
        self._hide_window(UI_CONNECTIONS)

    def show_audio_unit(self, au_id):
        self._show_window((UI_AUDIO_UNIT, au_id))

    def hide_audio_unit(self, au_id):
        self._hide_window((UI_AUDIO_UNIT, au_id))

    def hide_audio_unit_and_subdevices(self, au_id):
        self._hide_window((UI_AUDIO_UNIT, au_id))

        visible_entries = self._session.get_visible()

        au_prefix = '{}/'.format(au_id)
        au_entries = set(e for e in visible_entries
                if isinstance(e, tuple) and
                (e[0] == UI_AUDIO_UNIT) and
                e[1].startswith(au_prefix))
        for au_entry in au_entries:
            self._hide_window(au_entry)

        proc_prefix = '{}/'.format(au_id)
        proc_entries = set(e for e in visible_entries
                if isinstance(e, tuple) and
                (e[0] == UI_PROCESSOR) and
                e[1].startswith(proc_prefix))
        for proc_entry in proc_entries:
            self._hide_window(proc_entry)

    def show_songs_channels(self):
        self._show_window(UI_SONGS_CHS)

    def hide_songs_channels(self):
        self._hide_window(UI_SONGS_CHS)

    def show_env_and_bindings(self):
        self._show_window(UI_ENV_BIND)

    def hide_env_and_bindings(self):
        self._hide_window(UI_ENV_BIND)

    def show_processor(self, proc_id):
        self._show_window((UI_PROCESSOR, proc_id))

    def hide_processor(self, proc_id):
        self._hide_window((UI_PROCESSOR, proc_id))

    def show_grid_editor(self):
        self._show_window(UI_GRID_EDITOR)

    def hide_grid_editor(self):
        self._hide_window(UI_GRID_EDITOR)

    def show_notation_editor(self):
        self._show_window(UI_NOTATION)

    def hide_notation_editor(self):
        self._hide_window(UI_NOTATION)

    def show_tuning_table_editor(self, table_id):
        self._show_window((UI_TUNING_TABLE, table_id))

    def hide_tuning_table_editor(self, table_id):
        self._hide_window((UI_TUNING_TABLE, table_id))

    def show_general_module_settings(self):
        self._show_window(UI_GENERAL_MOD)

    def hide_general_module_settings(self):
        self._hide_window(UI_GENERAL_MOD)

    def show_settings(self):
        self._show_window(UI_SETTINGS)

    def hide_settings(self):
        self._hide_window(UI_SETTINGS)

    def show_render_stats(self):
        self._show_window(UI_RENDER_STATS)

    def hide_render_stats(self):
        self._hide_window(UI_RENDER_STATS)

    def show_interactivity_controls(self):
        self._show_window(UI_IA_CONTROLS)

    def hide_interactivity_controls(self):
        self._hide_window(UI_IA_CONTROLS)

    def get_visible(self):
        return self._session.get_visible()

    def get_and_clear_signalled(self):
        signalled = self._session.get_signalled_uis()
        self._session.clear_signalled_uis()
        return signalled


