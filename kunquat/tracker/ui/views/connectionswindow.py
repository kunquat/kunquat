# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2017
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

from .connectionseditor import ConnectionsEditor
from .keyboardmapper import KeyboardMapper
from .updater import Updater


class ConnectionsWindow(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._conns_editor = ConnectionsEditor()

        self._keyboard_mapper = KeyboardMapper()

        self.add_updating_child(self._conns_editor, self._keyboard_mapper)

        self.setWindowTitle('Connections')

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._conns_editor)
        self.setLayout(v)

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_connections()

    def sizeHint(self):
        return QSize(800, 600)

    def keyPressEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()

    def keyReleaseEvent(self, event):
        if not self._keyboard_mapper.process_typewriter_button_event(event):
            event.ignore()


