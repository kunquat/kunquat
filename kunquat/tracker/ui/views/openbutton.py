# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014
#          Toni Ruottu, Finland 2014
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

class OpenButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None

        self.setText('Open')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        pass

    def _clicked(self):
        module_path_qstring = QFileDialog.getOpenFileName(
                caption='Open Kunquat Composition',
                filter='Kunquat compositions (*.kqt *.kqt.gz *.kqt.bz2)')
        if module_path_qstring:
            module_path = str(module_path_qstring.toUtf8())
            process_manager = self._ui_model.get_process_manager()
            process_manager.new_kunquat(module_path)
