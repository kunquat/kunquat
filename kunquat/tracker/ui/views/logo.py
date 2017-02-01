# -*- coding: utf-8 -*-

#
# Authors: Toni Ruottu, Finland 2014
#          Tomi Jylhä-Ollila, Finland 2015-2017
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
from PySide.QtSvg import QSvgRenderer

from .updatingview import UpdatingView


class Logo(QWidget, UpdatingView):

    def __init__(self):
        super().__init__()
        self.setFixedSize(200, 200)

    def paintEvent(self, ev):
        if self._ui_model:
            logo_painter = QPainter(self)
            icon_bank = self._ui_model.get_icon_bank()
            logo_path = icon_bank.get_kunquat_logo_path()
            logo_renderer = QSvgRenderer(logo_path)
            logo_renderer.render(logo_painter)


