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

class TWLed(QFrame):

    def __init__(self, color_style):
        super(QFrame, self).__init__()
        self.setMinimumWidth(35)
        self.setMinimumHeight(15)
        self.setMaximumWidth(35)
        self.setMaximumHeight(15)
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.setLineWidth(2)
        self._color_style = color_style
        self._light = 8
        self._enabled = True

        self._left = QLabel()
        self._left.setMargin(0)
        self._left.setMaximumWidth(5)
        self._center = QLabel()
        self._center.setMargin(0)
        self._right = QLabel()
        self._right.setMargin(0)
        self._right.setMaximumWidth(5)

        h = QHBoxLayout()
        h.addWidget(self._left)
        h.addWidget(self._center)
        h.addWidget(self._right)
        h.setContentsMargins(0,0,0,0)
        h.setSpacing(0)
        self.setLayout(h)
        self._silent()

    def _flat(self):
        self._left.setStyleSheet("QLabel { background-color: #f00; }")
        self._center.setStyleSheet("QLabel { background-color: #500; }")
        self._right.setStyleSheet("QLabel { background-color: #400; }")

    def _exact(self):
        self._left.setStyleSheet("QLabel { background-color: #f00; }")
        self._center.setStyleSheet("QLabel { background-color: #f00; }")
        self._right.setStyleSheet("QLabel { background-color: #f00; }")

    def _sharp(self):
        self._left.setStyleSheet("QLabel { background-color: #400; }")
        self._center.setStyleSheet("QLabel { background-color: #500; }")
        self._right.setStyleSheet("QLabel { background-color: #f00; }")

    def _silent(self):
        self._left.setStyleSheet("QLabel { background-color: #400; }")
        self._center.setStyleSheet("QLabel { background-color: #400; }")
        self._right.setStyleSheet("QLabel { background-color: #400; }")

class TypeWriterButton(QPushButton):

    def __init__(self):
        QPushButton.__init__(self)
        self._instrument = None

        self.setMinimumWidth(60)
        self.setMinimumHeight(60)
        layout = QVBoxLayout(self)
        led = TWLed(100)
        self._led = led
        layout.addWidget(led)
        notename = QLabel()
        self._notename = notename
        notename.setAlignment(Qt.AlignCenter)
        layout.addWidget(notename)
        layout.setAlignment(Qt.AlignCenter)
        self.setFocusPolicy(Qt.NoFocus)

        self._notename.setText('100c')
        self.setStyleSheet("QLabel { background-color: #ffe; }")
        self._notename.setStyleSheet("QLabel { color: #000; }")


    def _play_sound(self):
        self._instrument.set_active_note(0, 100)

    def set_ui_model(self, ui_model):
        module = ui_model.get_module()
        self._instrument = module.get_instrument(0)
        QObject.connect(self, SIGNAL('clicked()'),
                        self._play_sound)

    def _update(self):
        pass

