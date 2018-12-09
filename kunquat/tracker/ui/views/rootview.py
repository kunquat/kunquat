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

from kunquat.tracker.ui.qt import *

import kunquat.tracker.cmdline as cmdline
from kunquat.tracker.ui.identifiers import *
from .stylecreator import StyleCreator
from .mainwindow import MainWindow
from .aboutwindow import AboutWindow
from .eventlistwindow import EventListWindow
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
from .updater import Updater
from . import utils


class RootView(Updater):

    def __init__(self):
        super().__init__()

        self._visible = set()

        self._style_creator = StyleCreator()
        self._crash_dialog = None

        self._main_window = MainWindow()
        self._single_windows = {}
        self._mult_windows = {
            UI_TUNING_TABLE:    {},
            UI_AUDIO_UNIT:      {},
            UI_PROCESSOR:       {},
        }

        self._module = None

        self._progress_window = None
        self._load_error_dialog = None
        self._au_import_error_dialog = None

    def _on_setup(self):
        self._style_creator.set_ui_model(self._ui_model)

        self._module = self._ui_model.get_module()

        self.register_action('signal_visibility', self._update_visibility)
        self.register_action('signal_audio_rendered', self._send_audio_state_queries)

        self.register_action('signal_module', self._on_module_setup_finished)
        self.register_action('signal_start_save_module', self._start_save_module)
        self.register_action(
                'signal_save_module_finished', self._on_save_module_finished)
        self.register_action('signal_start_import_au', self._start_import_au)
        self.register_action('signal_au_import_error', self._on_au_import_error)
        self.register_action('signal_au_import_finished', self._on_au_import_finished)
        self.register_action('signal_start_export_au', self._start_export_au)
        self.register_action('signal_export_au_finished', self._on_export_au_finished)
        self.register_action('signal_progress_start', self._show_progress_window)
        self.register_action('signal_progress_step', self._update_progress_window)
        self.register_action('signal_progress_finished', self._hide_progress_window)
        self.register_action('signal_module_load_error', self._on_module_load_error)
        self.register_action('signal_style_changed', self._update_style)

        style_mgr = self._ui_model.get_style_manager()
        style_mgr.set_init_style_sheet(QApplication.instance().styleSheet())
        style_sheet = self._style_creator.get_updated_style_sheet()
        QApplication.instance().setStyleSheet(style_sheet)

        self.add_to_updaters(self._main_window)

        self._update_visibility()

    def _on_teardown(self):
        self._style_creator.unregister_updaters()

    def _update_style(self):
        style_sheet = self._style_creator.get_updated_style_sheet()
        QApplication.instance().setStyleSheet(style_sheet)
        self._crash_dialog.update_style(self._ui_model.get_style_manager())

    def set_crash_dialog(self, crash_dialog):
        self._crash_dialog = crash_dialog
        self._crash_dialog.update_style(self._ui_model.get_style_manager())

    def show_main_window(self):
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.show_main()

    def setup_module(self):
        module = self._ui_model.get_module()
        module_path = cmdline.get_kqt_file()
        self._set_windows_enabled(False)
        task_executor = self._ui_model.get_task_executor()
        if module_path:
            module.set_path(module_path)
            module.execute_load(task_executor)
        else:
            module.execute_create_sandbox(task_executor)

    def _update_visibility(self):
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_update = visibility_mgr.get_visible()

        opened = visibility_update - self._visible
        closed = self._visible - visibility_update

        single_window_cons = {
            UI_ABOUT:           AboutWindow,
            UI_EVENT_LOG:       EventListWindow,
            UI_CONNECTIONS:     ConnectionsWindow,
            UI_SONGS_CHS:       SongsChannelsWindow,
            UI_NOTATION:        NotationWindow,
            UI_ENV_BIND:        EnvBindWindow,
            UI_GENERAL_MOD:     GeneralModWindow,
            UI_SETTINGS:        SettingsWindow,
            UI_GRID_EDITOR:     GridEditorWindow,
            UI_IA_CONTROLS:     IAWindow,
            UI_RENDER_STATS:    RenderStatsWindow,
        }

        mult_window_cons = {
            UI_TUNING_TABLE:    TuningTableWindow,
            UI_AUDIO_UNIT:      AuWindow,
            UI_PROCESSOR:       ProcWindow,
        }

        for ui in opened:
            # Check settings for UI visibility
            is_show_allowed = visibility_mgr.is_show_allowed()

            if ui == UI_MAIN:
                if is_show_allowed:
                    self._main_window.show()
            elif ui in single_window_cons:
                assert ui not in self._single_windows
                window = single_window_cons[ui]()
                self.add_to_updaters(window)
                self._single_windows[ui] = window
                if is_show_allowed:
                    window.show()
            elif isinstance(ui, tuple):
                ui_type, ui_id = ui
                assert ui_id not in self._mult_windows[ui_type]

                window = mult_window_cons[ui_type]()
                if ui_type == UI_TUNING_TABLE:
                    window.set_tuning_table_id(ui_id)
                elif ui_type == UI_AUDIO_UNIT:
                    window.set_au_id(ui_id)
                elif ui_type == UI_PROCESSOR:
                    proc_id = ui_id
                    proc_id_parts = proc_id.split('/')
                    au_id = '/'.join(proc_id_parts[:-1])
                    window.set_au_id(au_id)
                    window.set_proc_id(proc_id)
                else:
                    assert False

                self.add_to_updaters(window)
                self._mult_windows[ui_type][ui_id] = window
                if is_show_allowed:
                    window.show()
            else:
                raise ValueError('Unsupported UI type: {}'.format(ui))

        for ui in closed:
            if ui == UI_MAIN:
                self._main_window.hide()
            elif ui in single_window_cons:
                window = self._single_windows.pop(ui)
                self.remove_from_updaters(window)
                window.deleteLater()
            elif isinstance(ui, tuple):
                ui_type, ui_id = ui
                window = self._mult_windows[ui_type].pop(ui_id)
                self.remove_from_updaters(window)
                window.deleteLater()
            else:
                raise ValueError('Unsupported UI type: {}'.format(ui))

        self._visible = set(visibility_update)

        if self._visible:
            signalled = visibility_mgr.get_and_clear_signalled()

            # Raise signalled windows to top
            if signalled:
                for signalled_id in signalled:
                    if isinstance(signalled_id, tuple):
                        win_type, win_id = signalled_id
                        window = self._mult_windows[win_type].get(win_id)
                    else:
                        window = self._single_windows.get(signalled_id)

                    if window:
                        window.activateWindow()
                        window.raise_()

        else:
            QApplication.quit()

    def _send_audio_state_queries(self):
        self._ui_model.clock()

    def _show_progress_window(self):
        assert not self._progress_window
        self._progress_window = ProgressWindow(self._ui_model)

        stat_mgr = self._ui_model.get_stat_manager()
        self._progress_window.set_description(stat_mgr.get_progress_description())
        self._progress_window.set_progress_norm(stat_mgr.get_progress_norm())

        visibility_mgr = self._ui_model.get_visibility_manager()
        if visibility_mgr.is_show_allowed():
            self._progress_window.delayed_show()

    def _update_progress_window(self):
        assert self._progress_window
        stat_mgr = self._ui_model.get_stat_manager()
        self._progress_window.set_description(stat_mgr.get_progress_description())
        self._progress_window.set_progress_norm(stat_mgr.get_progress_norm())

    def _hide_progress_window(self):
        assert self._progress_window
        self._progress_window.deleteLater()
        self._progress_window = None

    def _on_module_setup_finished(self):
        self._set_windows_enabled(True)

    def _set_windows_enabled(self, enabled):
        def try_set_enabled(window):
            if window:
                window.setEnabled(enabled)

        self._main_window.setEnabled(enabled)

        for window in self._single_windows.values():
            window.setEnabled(enabled)

        for group in self._mult_windows.values():
            for window in group.values():
                window.setEnabled(enabled)

    def _start_save_module(self):
        self._set_windows_enabled(False)
        self._module.flush(self._execute_save_module)

    def _execute_save_module(self):
        task_executor = self._ui_model.get_task_executor()
        self._module.execute_save(task_executor)

    def _on_save_module_finished(self):
        self._module.finish_save()
        self._set_windows_enabled(True)

    def _start_import_au(self):
        self._set_windows_enabled(False)
        task_executor = self._ui_model.get_task_executor()
        self._module.execute_import_au(task_executor)

    def _on_module_load_error(self):
        def on_close():
            if self._load_error_dialog:
                self._load_error_dialog.close()
                self._load_error_dialog = None
                visibility_mgr = self._ui_model.get_visibility_manager()
                visibility_mgr.hide_all()

        error_info = self._module.get_load_error_info()
        assert error_info
        self._load_error_dialog = ModuleLoadErrorDialog(
                self._ui_model, error_info, on_close)
        self._load_error_dialog.setModal(True)
        self._load_error_dialog.show()

    def _on_au_import_error(self):
        def on_close():
            if self._au_import_error_dialog:
                self._au_import_error_dialog.close()
                self._au_import_error_dialog = None

        error_info = self._module.get_reset_au_import_error_info()
        assert error_info
        self._au_import_error_dialog = AuImportErrorDialog(
                self._ui_model, error_info, on_close)
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
        task_executor = self._ui_model.get_task_executor()
        self._module.execute_export_au(task_executor)

    def _on_export_au_finished(self):
        self._module.finish_export_au()
        self._set_windows_enabled(True)


class ProgressWindow(QWidget):

    _PROGRESS_STEP_COUNT = 10000
    _SHOW_DELAY = 0.2

    def __init__(self, ui_model):
        super().__init__()
        style_mgr = ui_model.get_style_manager()

        width = utils.get_abs_window_size(0.3, 0.5).width()
        self.setMinimumWidth(width)
        self.setWindowModality(Qt.ApplicationModal)
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Minimum)

        self._desc = QLabel()
        self._desc.setWordWrap(True)
        self._progress = QProgressBar()
        self._progress.setMaximum(self._PROGRESS_STEP_COUNT)
        self._progress.setMinimum(0)

        v = QVBoxLayout()
        margin = style_mgr.get_scaled_size_param('large_padding')
        v.setContentsMargins(margin, margin, margin, margin)
        v.setSpacing(style_mgr.get_scaled_size_param('medium_padding'))
        v.addWidget(self._desc, Qt.AlignLeft)
        v.addWidget(self._progress)
        self.setLayout(v)

        self._show_timer = QTimer()
        self._show_timer.setSingleShot(True)
        self._show_timer.timeout.connect(self.show)

        self.hide()

    def delayed_show(self):
        self._show_timer.start(self._SHOW_DELAY * 1000)

    def set_description(self, desc):
        self._desc.setText(desc)

    def set_progress_norm(self, progress_norm):
        if progress_norm == None:
            self._progress.setMaximum(0)
        else:
            self._progress.setMaximum(self._PROGRESS_STEP_COUNT)
            self._progress.setValue(int(progress_norm * self._PROGRESS_STEP_COUNT))

    def closeEvent(self, event):
        event.ignore()


class ImportErrorDialog(QDialog):

    def __init__(self, ui_model, title, msg_fmt, error_info, on_close):
        super().__init__()
        style_mgr = ui_model.get_style_manager()
        icon_bank = ui_model.get_icon_bank()

        self.setWindowTitle(title)

        self._on_close = on_close

        path, details = error_info

        error_img_orig = QPixmap(icon_bank.get_icon_path('error'))
        error_img = error_img_orig.scaledToWidth(
                style_mgr.get_scaled_size_param('dialog_icon_size'),
                Qt.SmoothTransformation)
        error_label = QLabel()
        error_label.setPixmap(error_img)

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        margin = style_mgr.get_scaled_size_param('large_padding')
        h.setContentsMargins(margin, margin, margin, margin)
        h.setSpacing(margin * 2)
        h.addWidget(error_label)
        h.addWidget(self._message)

        self._button_layout = QHBoxLayout()

        v = QVBoxLayout()
        v.addLayout(h)
        v.addLayout(self._button_layout)

        self.setLayout(v)

        # Dialog contents
        detail_lines = details.split('\n')
        format_detail_lines = (
                '<p style="margin-top: 0.2em; margin-bottom: 0.2em;">{}</p>'.format(d)
                for d in detail_lines)
        format_details = ' '.join(format_detail_lines)
        error_msg = ('<p>{}</p> {}'.format(msg_fmt.format(path), format_details))
        self._message.setText(error_msg)

        ok_button = QPushButton('OK')
        self._button_layout.addStretch(1)
        self._button_layout.addWidget(ok_button)
        self._button_layout.addStretch(1)

        ok_button.clicked.connect(self.close)

    def closeEvent(self, event):
        self._on_close()


class ModuleLoadErrorDialog(ImportErrorDialog):

    def __init__(self, ui_model, error_info, on_close):
        super().__init__(
                ui_model,
                'Module loading failed',
                'Could not load \'{}\' due to the following error:',
                error_info,
                on_close)


class AuImportErrorDialog(ImportErrorDialog):

    def __init__(self, ui_model, error_info, on_close):
        super().__init__(
                ui_model,
                'Importing failed',
                'Could not import \'{}\' due to the following error:',
                error_info,
                on_close)


