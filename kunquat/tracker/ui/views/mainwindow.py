# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2016
#          Toni Ruottu, Finland 2013-2014
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
from .confirmdialog import ConfirmDialog
from .mainview import MainView
from .saving import get_module_save_path


class MainWindow(QWidget):

    def __init__(self):
        super().__init__()
        self.setWindowTitle('Kunquat Tracker')
        self._ui_model = None
        self._updater = None

        self._quit_after_saving = False

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

    def update_icon(self):
        icon_bank = self._ui_model.get_icon_bank()
        icon_path = icon_bank.get_kunquat_logo_path()
        icon = QIcon(icon_path)
        self.setWindowIcon(icon)

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)
        self._main_view.unregister_updaters()

    def _perform_updates(self, signals):
        if self._quit_after_saving and ('signal_save_module_finished' in signals):
            visibility_manager = self._ui_model.get_visibility_manager()
            visibility_manager.hide_main_after_saving()

    def _perform_save_and_close(self):
        module = self._ui_model.get_module()

        if not module.get_path():
            module_path = get_module_save_path()
            if not module_path:
                return
            module.set_path(module_path)

        self._quit_after_saving = True

        module.start_save()

    def _perform_discard_and_close(self):
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_main()

    def closeEvent(self, event):
        event.ignore()

        module = self._ui_model.get_module()
        if module.is_modified():
            dialog = ExitUnsavedConfirmDialog(
                    self._ui_model.get_icon_bank(),
                    self._perform_save_and_close,
                    self._perform_discard_and_close)
            dialog.exec_()
        else:
            visibility_manager = self._ui_model.get_visibility_manager()
            visibility_manager.hide_main()

    def sizeHint(self):
        return QSize(1024, 768)


class ExitUnsavedConfirmDialog(ConfirmDialog):

    def __init__(self, icon_bank, action_save, action_discard):
        super().__init__(icon_bank)

        self._action_save = action_save
        self._action_discard = action_discard

        self.setWindowTitle('Unsaved changes')

        msg = 'There are unsaved changes in the project.'
        self._set_message(msg)

        self._discard_button = QPushButton('Discard changes and exit')
        self._cancel_button = QPushButton('Keep the project open')
        self._save_button = QPushButton('Save changes and exit')

        b = self._get_button_layout()
        b.addWidget(self._discard_button)
        b.addWidget(self._cancel_button)
        b.addWidget(self._save_button)

        self._cancel_button.setFocus(Qt.PopupFocusReason)

        QObject.connect(self._save_button, SIGNAL('clicked()'), self._perform_save)
        QObject.connect(self._discard_button, SIGNAL('clicked()'), self._perform_discard)
        QObject.connect(self._cancel_button, SIGNAL('clicked()'), self.close)

    def _perform_save(self):
        self._action_save()
        self.close()

    def _perform_discard(self):
        self._action_discard()
        self.close()


