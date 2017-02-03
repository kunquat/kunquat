# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2017
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

from .name import Name
from .processorupdater import ProcessorUpdater


class InfoEditor(QWidget, ProcessorUpdater):

    def __init__(self):
        super().__init__()
        self._name = Name()

        self.add_to_updaters(self._name)

        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._name, 0, Qt.AlignTop)
        self.setLayout(v)


