# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from config import *
from utils import *
import tstamp


class View(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_config(self, config):
        self._config = config

    def paintEvent(self, ev):
        start = time.time()

        painter = QPainter(self)

        # Testing
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, self.width(), self.height()))
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self.width() - 1, self.height() - 1)

        end = time.time()
        elapsed = end - start
        print('View updated in {:.2f} ms'.format(elapsed * 1000))


