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

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.views.updater import Updater
from .sheetarea import SheetArea
from .toolbar import Toolbar


class Sheet(QWidget, Updater):

    def __init__(self):
        super().__init__()
        self._toolbar = Toolbar()
        self._sheet_area = SheetArea()

        self.add_to_updaters(self._toolbar, self._sheet_area)

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 4)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._sheet_area)
        self.setLayout(v)

    def minimumSizeHint(self):
        return QSize(self._toolbar.minimumSizeHint().width(), 0)


