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
        if self._is_closing:
            return
        self._session.show_ui(UI_MAIN)
        self._updater.signal_update()

    def hide_main(self):
        self._session.hide_ui(UI_MAIN)
        self._updater.signal_update()

    def hide_main_after_saving(self):
        # XXX: This hacky version can be called from an update function
        #      It only works because there are always some signals emitted
        self._session.hide_ui(UI_MAIN)

    def set_input_control_view(self, mode):
        config = get_config()
        config.set_value('input_control_view', mode)

    def get_input_control_view(self):
        config = get_config()
        return config.get_value('input_control_view')

    def _show_window(self, ui):
        if self._is_closing:
            return
        self._session.show_ui(ui)

        if type(ui) == str:
            signal = 'signal_window_{}'.format(ui)
        else:
            signal = 'signal_window_{}_{}'.format(ui[0], ui[1])
        self._updater.signal_update(signal)

    def show_about(self):
        self._show_window(UI_ABOUT)

    def hide_about(self):
        self._session.hide_ui(UI_ABOUT)
        self._updater.signal_update()

    def show_event_log(self):
        self._show_window(UI_EVENT_LOG)

    def hide_event_log(self):
        self._session.hide_ui(UI_EVENT_LOG)
        self._updater.signal_update()

    def show_connections(self):
        self._show_window(UI_CONNECTIONS)

    def hide_connections(self):
        self._session.hide_ui(UI_CONNECTIONS)
        self._updater.signal_update()

    def show_audio_unit(self, au_id):
        self._show_window((UI_AUDIO_UNIT, au_id))

    def hide_audio_unit(self, au_id):
        self._session.hide_ui((UI_AUDIO_UNIT, au_id))
        self._updater.signal_update()

    def hide_audio_unit_and_processors(self, au_id):
        self._session.hide_ui((UI_AUDIO_UNIT, au_id))

        proc_entries = set()
        prefix = '{}/'.format(au_id)
        visible_entries = self._session.get_visible()
        for entry in visible_entries:
            if (isinstance(entry, tuple) and
                    (entry[0] == UI_PROCESSOR) and
                    entry[1].startswith(prefix)):
                proc_entries.add(entry)
        for proc_entry in proc_entries:
            self._session.hide_ui(proc_entry)

        self._updater.signal_update()

    def show_songs_channels(self):
        self._show_window(UI_SONGS_CHS)

    def hide_songs_channels(self):
        self._session.hide_ui(UI_SONGS_CHS)
        self._updater.signal_update()

    def show_env_and_bindings(self):
        self._show_window(UI_ENV_BIND)

    def hide_env_and_bindings(self):
        self._session.hide_ui(UI_ENV_BIND)
        self._updater.signal_update()

    def show_processor(self, proc_id):
        self._show_window((UI_PROCESSOR, proc_id))

    def hide_processor(self, proc_id):
        self._session.hide_ui((UI_PROCESSOR, proc_id))
        self._updater.signal_update()

    def show_grid_editor(self):
        self._show_window(UI_GRID_EDITOR)

    def hide_grid_editor(self):
        self._session.hide_ui(UI_GRID_EDITOR)
        self._updater.signal_update()

    def show_notation_editor(self):
        self._show_window(UI_NOTATION)

    def hide_notation_editor(self):
        self._session.hide_ui(UI_NOTATION)
        self._updater.signal_update()

    def show_tuning_table_editor(self, table_id):
        self._show_window((UI_TUNING_TABLE, table_id))

    def hide_tuning_table_editor(self, table_id):
        self._session.hide_ui((UI_TUNING_TABLE, table_id))
        self._updater.signal_update()

    def show_general_module_settings(self):
        self._show_window(UI_GENERAL_MOD)

    def hide_general_module_settings(self):
        self._session.hide_ui(UI_GENERAL_MOD)
        self._updater.signal_update()

    def show_settings(self):
        self._show_window(UI_SETTINGS)

    def hide_settings(self):
        self._session.hide_ui(UI_SETTINGS)
        self._updater.signal_update()

    def show_render_stats(self):
        self._show_window(UI_RENDER_STATS)

    def hide_render_stats(self):
        self._session.hide_ui(UI_RENDER_STATS)
        self._updater.signal_update()

    def show_interactivity_controls(self):
        self._show_window(UI_IA_CONTROLS)

    def hide_interactivity_controls(self):
        self._session.hide_ui(UI_IA_CONTROLS)
        self._updater.signal_update()

    def get_visible(self):
        return self._session.get_visible()


