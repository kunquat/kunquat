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

from typewriterbutton import TypeWriterButton

class TypeWriter(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._type_writer_button1 = TypeWriterButton(100)
        self._type_writer_button2 = TypeWriterButton(200)
        self._type_writer_button3 = TypeWriterButton(300)
        self._type_writer_button4 = TypeWriterButton(400)
        self._type_writer_button5 = TypeWriterButton(500)
        self._type_writer_button6 = TypeWriterButton(600)
        self._type_writer_button7 = TypeWriterButton(700)
        self._type_writer_button8 = TypeWriterButton(800)

        h = QHBoxLayout()
        h.addWidget(self._type_writer_button1)
        h.addWidget(self._type_writer_button2)
        h.addWidget(self._type_writer_button3)
        h.addWidget(self._type_writer_button4)
        h.addWidget(self._type_writer_button5)
        h.addWidget(self._type_writer_button6)
        h.addWidget(self._type_writer_button7)
        h.addWidget(self._type_writer_button8)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._type_writer_button1.set_ui_model(ui_model) 
        self._type_writer_button2.set_ui_model(ui_model)         
        self._type_writer_button3.set_ui_model(ui_model) 
        self._type_writer_button4.set_ui_model(ui_model) 
        self._type_writer_button5.set_ui_model(ui_model) 
        self._type_writer_button6.set_ui_model(ui_model) 
        self._type_writer_button7.set_ui_model(ui_model) 
        self._type_writer_button8.set_ui_model(ui_model) 
