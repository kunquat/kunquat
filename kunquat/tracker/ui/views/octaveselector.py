# -*- coding: utf-8 -*-

#
# Author: Toni Ruottu, Finland 2013
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

from octavebutton import OctaveButton

class OctaveSelector(QAbstractScrollArea):

    def __init__(self):
        QAbstractScrollArea.__init__(self)
        self.setFocusPolicy(Qt.TabFocus)
        self._ui_model = None
        self._typewriter_manager = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._typewriter_manager = ui_model.get_typewriter_manager()
        self._update()

    def _update(self):
        view = self._get_view()
        self.setViewport(view)

    def _get_view(self):
        octave_count = self._typewriter_manager.get_octave_count()
        view = QWidget()
        row = QHBoxLayout(view)
        for i in range(octave_count):
            button = self.get_button(i)
            row.addWidget(button)
        row.addStretch(1)
        return view

    def get_button(self, octave_id):
        button = OctaveButton(octave_id)
        button.set_ui_model(self._ui_model)
        return button


