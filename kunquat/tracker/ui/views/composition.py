# -*- coding: utf-8 -*-

#
# Authors: Tomi Jylhä-Ollila, Finland 2013-2014
#          Toni Ruottu, Finland 2013-2014
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

from playbutton import PlayButton
from sheet.sheet import Sheet


class Composition(QWidget):

    def __init__(self):
        QWidget.__init__(self)
        self._ui_model = None
        self._play_button = PlayButton()
        self._sheet = Sheet()

        v = QVBoxLayout()
        v.addWidget(self._play_button)
        v.addWidget(self._sheet)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._play_button.set_ui_model(ui_model)
        self._sheet.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._play_button.unregister_updaters()
        self._sheet.unregister_updaters()

