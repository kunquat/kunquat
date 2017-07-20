# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2017
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

from .exiting import ExitHelper
from .mainview import MainView
from .saverwindow import SaverWindow


class MainWindow(SaverWindow):

    def __init__(self):
        super().__init__()
        self.setWindowTitle('Kunquat Tracker')
        self._ui_model = None
        self._updater = None

        self._exit_helper = ExitHelper()

        self._main_view = MainView()
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self._main_view)
        self.setLayout(layout)

        self.hide()

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._main_view.set_ui_model(ui_model)
        self.update_icon()
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._exit_helper.set_ui_model(ui_model)

    def update_icon(self):
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_kunquat_logo_path()
        icon = QIcon(icon_path)
        self.setWindowIcon(icon)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._main_view.unregister_updaters()

    def _perform_updates(self, signals):
        if 'signal_save_module_finished' in signals:
            self._exit_helper.notify_save_module_finished()

    def closeEvent(self, event):
        event.ignore()
        self._exit_helper.try_exit()

    def sizeHint(self):
        return QSize(1280, 768)


