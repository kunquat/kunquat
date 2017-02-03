# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
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

from .tuningtableeditor import TuningTableEditor
from .updater import Updater


class TuningTableWindow(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._table_id = None
        self._ui_model = None

        self._editor = TuningTableEditor()

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.addWidget(self._editor)
        self.setLayout(v)

    def set_tuning_table_id(self, table_id):
        self._table_id = table_id
        self._editor.set_tuning_table_id(table_id)

    def _on_setup(self):
        assert self._table_id != None
        self.add_to_updaters(self._editor)
        self.register_action('signal_tuning_tables', self._update_title)

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


