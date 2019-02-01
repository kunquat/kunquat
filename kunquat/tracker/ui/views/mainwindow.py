# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2019
#          Toni Ruottu, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *

from kunquat.tracker.version import KUNQUAT_VERSION
from .exiting import ExitHelper
from .mainview import MainView
from .saverwindow import SaverWindow
from .updater import Updater
from .utils import get_abs_window_size


class MainWindow(Updater, SaverWindow):

    def __init__(self):
        super().__init__()
        self.setWindowTitle('Kunquat Tracker')
        self._ui_model = None
        self._updater = None

        self._exit_helper = ExitHelper()

        self._main_view = MainView()

        self.add_to_updaters(self._main_view)

        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self._main_view)
        self.setLayout(layout)

        self.hide()

    def _on_setup(self):
        self._exit_helper.set_ui_model(self._ui_model)

        self.register_action(
                'signal_module_save_error', self._exit_helper.notify_save_module_error)
        self.register_action(
                'signal_save_module_finished',
                self._exit_helper.notify_save_module_finished)
        self.register_action('signal_module', self._update_title)
        self.register_action('signal_store', self._update_title)

        self._update_icon()
        self._update_title()

    def _update_icon(self):
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_kunquat_logo_path()
        icon = QIcon(icon_path)
        self.setWindowIcon(icon)

    def _update_title(self):
        tracker_str = 'Kunquat Tracker'
        if KUNQUAT_VERSION:
            tracker_str += ' {}'.format(KUNQUAT_VERSION)

        module = self._ui_model.get_module()
        name = module.get_name()
        title_str = name or 'Untitled'
        if module.is_modified():
            title_str = '*' + title_str

        window_title_str = '{} – {}'.format(title_str, tracker_str)
        self.setWindowTitle(window_title_str)

    def closeEvent(self, event):
        event.ignore()
        self._exit_helper.try_exit()

    def sizeHint(self):
        return get_abs_window_size(0.75, 0.75)


