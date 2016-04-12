# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
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

from .tuningtableeditor import TuningTableEditor


class TuningTableWindow(QWidget):

    def __init__(self):
        super().__init__()
        self._table_id = None
        self._ui_model = None

        self._editor = TuningTableEditor()

        v = QVBoxLayout()
        v.setMargin(4)
        v.addWidget(self._editor)
        self.setLayout(v)

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id
        self._editor.set_tuning_table_id(table_id)

    def set_ui_model(self, ui_model):
        assert self._table_id != None
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)
        self._editor.set_ui_model(ui_model)

        self._update_title()

    def unregister_updaters(self):
        self._editor.unregister_updaters()
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_tuning_tables' in signals:
            self._update_title()

    def _update_title(self):
        module = self._ui_model.get_module()
        table = module.get_tuning_table(self._table_id)
        name = table.get_name()
        if name:
            title = '{} – Kunquat Tracker'.format(name)
        else:
            title = 'Kunquat Tracker'
        self.setWindowTitle(title)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_tuning_table_editor(self._table_id)

    def sizeHint(self):
        return QSize(480, 640)


