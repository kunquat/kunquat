# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2014
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

from lib.task_executor import execute_task


class SaveButton(QToolButton):

    def __init__(self):
        QToolButton.__init__(self)
        self._ui_model = None

        self.setText('Save')

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def unregister_updaters(self):
        pass

    def _clicked(self):
        module = self._ui_model.get_module()
        module.save(execute_task)


