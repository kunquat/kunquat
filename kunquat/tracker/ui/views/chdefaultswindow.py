# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
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

from chdefaultseditor import ChDefaultsEditor


class ChDefaultsWindow(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._ch_defaults_editor = ChDefaultsEditor()

        self.setWindowTitle('Channel defaults')

        v = QVBoxLayout()
        v.setMargin(0)
        v.setSpacing(0)
        v.addWidget(self._ch_defaults_editor)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._ch_defaults_editor.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._ch_defaults_editor.unregister_updaters()

    def closeEvent(self, event):
        event.ignore()
        visibility_manager = self._ui_model.get_visibility_manager()
        visibility_manager.hide_ch_defaults()

    def sizeHint(self):
        return QSize(512, 768)


