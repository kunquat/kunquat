# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014-2016
#          Toni Ruottu, Finland 2014
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

import kunquat.tracker.cmdline as cmdline
from kunquat.tracker.ui.identifiers import *
from .stylecreator import StyleCreator
from .mainwindow import MainWindow
from .aboutwindow import AboutWindow
from .eventlist import EventList
from .connectionswindow import ConnectionsWindow
from .songschannelswindow import SongsChannelsWindow
from .notationwindow import NotationWindow
from .tuningtablewindow import TuningTableWindow
from .envbindwindow import EnvBindWindow
from .generalmodwindow import GeneralModWindow
from .settingswindow import SettingsWindow
from .auwindow import AuWindow
from .procwindow import ProcWindow
from .sheet.grideditorwindow import GridEditorWindow
from .iawindow import IAWindow
from .renderstatswindow import RenderStatsWindow


class RootView():

    def __init__(self):
        self._ui_model = None
        self._task_executer = None
        self._updater = None
        self._visible = set()

        self._style_creator = StyleCreator()
        self._main_window = MainWindow()
        self._about_window = None
        self._event_log = None
        self._connections = None
        self._songs_channels = None
        self._notation = None
        self._tuning_tables = {}
        self._env_bind = None
        self._general_mod = None
        self._settings = None
        self._au_windows = {}
        self._proc_windows = {}
        self._grid_editor = None
        self._ia_controls = None
        self._render_stats = None

        self._module = None
        self._au_import_error_dialog = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

        self._style_creator.set_ui_model(ui_model)
        self._main_window.set_ui_model(ui_model)
        self._updater = self._ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._module = self._ui_model.get_module()

        style_manager = self._ui_model.get_style_manager()
        style_manager.set_default_style_sheet(QApplication.instance().styleSheet())
        self._style_creator.update_style_sheet()

    def show_main_window(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.show_main()

    def setup_module(self):
        module = self._ui_model.get_module()
        module_path = cmdline.get_kqt_file()
        if module_path:
            module.set_path(module_path)
            module.execute_load(self._task_executer)
        else:
            module.execute_create_sandbox(self._task_executer)

    def set_task_executer(self, task_executer):
        self._task_executer = task_executer

    def _perform_updates(self, signals):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_update = visibility_manager.get_visible()

        opened = visibility_update - self._visible
        closed = self._visible - visibility_update

        for ui in opened:
            # Check settings for UI visibility
            is_show_allowed = visibility_manager.is_show_allowed()

            if ui == UI_MAIN:
                if is_show_allowed:
                    self._main_window.show()
            elif ui == UI_ABOUT:
                self._about_window = AboutWindow()
                self._about_window.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._about_window.show()
            elif ui == UI_EVENT_LOG:
                self._event_log = EventList()
                self._event_log.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._event_log.show()
            elif ui == UI_CONNECTIONS:
                self._connections = ConnectionsWindow()
                self._connections.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._connections.show()
            elif ui == UI_SONGS_CHS:
                self._songs_channels = SongsChannelsWindow()
                self._songs_channels.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._songs_channels.show()
            elif ui == UI_NOTATION:
                self._notation = NotationWindow()
                self._notation.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._notation.show()
            elif type(ui) == tuple and ui[0] == UI_TUNING_TABLE:
                table_id = ui[1]
                tt_window = TuningTableWindow()
                tt_window.set_tuning_table_id(table_id)
                tt_window.set_ui_model(self._ui_model)
                self._tuning_tables[table_id] = tt_window
                if is_show_allowed:
                    self._tuning_tables[table_id].show()
            elif ui == UI_ENV_BIND:
                self._env_bind = EnvBindWindow()
                self._env_bind.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._env_bind.show()
            elif ui == UI_GENERAL_MOD:
                self._general_mod = GeneralModWindow()
                self._general_mod.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._general_mod.show()
            elif ui == UI_SETTINGS:
                self._settings = SettingsWindow()
                self._settings.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._settings.show()
            elif type(ui) == tuple and ui[0] == UI_AUDIO_UNIT:
                au_id = ui[1]
                au_window = AuWindow()
                au_window.set_au_id(au_id)
                au_window.set_ui_model(self._ui_model)
                self._au_windows[au_id] = au_window
                if is_show_allowed:
                    self._au_windows[au_id].show()
            elif type(ui) == tuple and ui[0] == UI_PROCESSOR:
                proc_id = ui[1]
                proc_id_parts = proc_id.split('/')
                au_id = '/'.join(proc_id_parts[:-1])
                proc_window = ProcWindow()
                proc_window.set_au_id(au_id)
                proc_window.set_proc_id(proc_id)
                proc_window.set_ui_model(self._ui_model)
                self._proc_windows[proc_id] = proc_window
                if is_show_allowed:
                    self._proc_windows[proc_id].show()
            elif ui == UI_GRID_EDITOR:
                self._grid_editor = GridEditorWindow()
                self._grid_editor.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._grid_editor.show()
            elif ui == UI_IA_CONTROLS:
                self._ia_controls = IAWindow()
                self._ia_controls.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._ia_controls.show()
            elif ui == UI_RENDER_STATS:
                self._render_stats = RenderStatsWindow()
                self._render_stats.set_ui_model(self._ui_model)
                if is_show_allowed:
                    self._render_stats.show()
            else:
                raise ValueError('Unsupported UI type: {}'.format(ui))

        for ui in closed:
            if ui == UI_MAIN:
                visibility_manager.hide_all()
                self._main_window.hide()
            elif ui == UI_ABOUT:
                self._about_window.unregister_updaters()
                self._about_window.deleteLater()
                self._about_window = None
            elif ui == UI_EVENT_LOG:
                self._event_log.unregister_updaters()
                self._event_log.deleteLater()
                self._event_log = None
            elif ui == UI_CONNECTIONS:
                self._connections.unregister_updaters()
                self._connections.deleteLater()
                self._connections = None
            elif ui == UI_SONGS_CHS:
                self._songs_channels.unregister_updaters()
                self._songs_channels.deleteLater()
                self._songs_channels = None
            elif ui == UI_NOTATION:
                self._notation.unregister_updaters()
                self._notation.deleteLater()
                self._notation = None
            elif type(ui) == tuple and ui[0] == UI_TUNING_TABLE:
                table_id = ui[1]
                tt_window = self._tuning_tables.pop(table_id)
                tt_window.unregister_updaters()
                tt_window.deleteLater()
            elif ui == UI_ENV_BIND:
                self._env_bind.unregister_updaters()
                self._env_bind.deleteLater()
                self._env_bind = None
            elif ui == UI_GENERAL_MOD:
                self._general_mod.unregister_updaters()
                self._general_mod.deleteLater()
                self._general_mod = None
            elif ui == UI_SETTINGS:
                self._settings.unregister_updaters()
                self._settings.deleteLater()
                self._settings = None
            elif type(ui) == tuple and ui[0] == UI_AUDIO_UNIT:
                au_id = ui[1]
                au_window = self._au_windows.pop(au_id)
                au_window.unregister_updaters()
                au_window.deleteLater()
            elif type(ui) == tuple and ui[0] == UI_PROCESSOR:
                proc_id = ui[1]
                proc_window = self._proc_windows.pop(proc_id)
                proc_window.unregister_updaters()
                proc_window.deleteLater()
            elif ui == UI_GRID_EDITOR:
                self._grid_editor.unregister_updaters()
                self._grid_editor.deleteLater()
                self._grid_editor = None
            elif ui == UI_IA_CONTROLS:
                self._ia_controls.unregister_updaters()
                self._ia_controls.deleteLater()
                self._ia_controls = None
            elif ui == UI_RENDER_STATS:
                self._render_stats.unregister_updaters()
                self._render_stats.deleteLater()
                self._render_stats = None
            else:
                raise ValueError('Unsupported UI type: {}'.format(ui))

        self._visible = set(visibility_update)

        if self._visible:
            if 'signal_start_save_module' in signals:
                self._start_save_module()
            if 'signal_save_module_finished' in signals:
                self._on_save_module_finished()
            if 'signal_start_import_au' in signals:
                self._start_import_au()
            if 'signal_au_import_error' in signals:
                self._on_au_import_error()
            if 'signal_au_import_finished' in signals:
                self._on_au_import_finished()
            if 'signal_start_export_au' in signals:
                self._start_export_au()
            if 'signal_export_au_finished' in signals:
                self._on_export_au_finished()
        else:
            QApplication.quit()

        self._ui_model.clock()

    def _set_windows_enabled(self, enabled):
        def try_set_enabled(window):
            if window:
                window.setEnabled(enabled)

        try_set_enabled(self._main_window)
        try_set_enabled(self._about_window)
        try_set_enabled(self._event_log)
        try_set_enabled(self._connections)
        try_set_enabled(self._songs_channels)
        try_set_enabled(self._notation)
        for window in self._tuning_tables.values():
            window.setEnabled(enabled)
        try_set_enabled(self._env_bind)
        try_set_enabled(self._general_mod)
        try_set_enabled(self._settings)
        for window in self._au_windows.values():
            window.setEnabled(enabled)
        for window in self._proc_windows.values():
            window.setEnabled(enabled)
        try_set_enabled(self._grid_editor)
        try_set_enabled(self._ia_controls)
        try_set_enabled(self._render_stats)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._main_window.unregister_updaters()
        self._style_creator.unregister_updaters()

    def _start_save_module(self):
        self._set_windows_enabled(False)
        self._module.flush(self._execute_save_module)

    def _execute_save_module(self):
        self._module.execute_save(self._task_executer)

    def _on_save_module_finished(self):
        self._module.finish_save()
        self._set_windows_enabled(True)

    def _start_import_au(self):
        self._set_windows_enabled(False)
        self._module.execute_import_au(self._task_executer)

    def _on_au_import_error(self):
        def on_close():
            if self._au_import_error_dialog:
                self._au_import_error_dialog.close()
                self._au_import_error_dialog = None

        error_info = self._module.get_reset_au_import_error_info()
        assert error_info
        self._au_import_error_dialog = AuImportErrorDialog(
                self._ui_model.get_icon_bank(), error_info, on_close)
        self._au_import_error_dialog.setModal(True)
        self._au_import_error_dialog.show()

    def _on_au_import_finished(self):
        module = self._ui_model.get_module()
        module.finish_import_au()
        self._set_windows_enabled(True)

    def _start_export_au(self):
        self._set_windows_enabled(False)
        self._module.flush(self._execute_export_au)

    def _execute_export_au(self):
        self._module.execute_export_au(self._task_executer)

    def _on_export_au_finished(self):
        self._module.finish_export_au()
        self._set_windows_enabled(True)


class AuImportErrorDialog(QDialog):

    def __init__(self, icon_bank, error_info, on_close):
        super().__init__()

        self.setWindowTitle('Importing failed')

        self._on_close = on_close

        path, details = error_info

        error_img_path = icon_bank.get_icon_path('error')
        error_label = QLabel()
        error_label.setPixmap(QPixmap(error_img_path))

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        h.setContentsMargins(8, 8, 8, 8)
        h.setSpacing(16)
        h.addWidget(error_label)
        h.addWidget(self._message)

        self._button_layout = QHBoxLayout()

        v = QVBoxLayout()
        v.addLayout(h)
        v.addLayout(self._button_layout)

        self.setLayout(v)

        # Dialog contents
        error_msg = ('<p>Could not import \'{}\' due to the following error:</p>'
            '<p>{}</p>'.format(path, details))
        self._message.setText(error_msg)

        ok_button = QPushButton('OK')
        self._button_layout.addStretch(1)
        self._button_layout.addWidget(ok_button)
        self._button_layout.addStretch(1)

        QObject.connect(ok_button, SIGNAL('clicked()'), self._on_close)


