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

from kunquat.kunquat.limits import *
from .kqtutils import get_kqt_file_path, open_kqt_au


class OpenButton(QToolButton):

    def __init__(self):
        super().__init__()
        self._ui_model = None

        self.setText('Open')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        pass

    def _clicked(self):
        file_path = get_kqt_file_path(set(['kqt', 'kqti', 'kqte']))
        if file_path:
            if file_path.endswith(('.kqt', '.kqt.gz', 'kqt.bz2')):
                process_manager = self._ui_model.get_process_manager()
                process_manager.new_kunquat(file_path)
            else:
                open_kqt_au(file_path, self._ui_model, self._ui_model.get_module())


