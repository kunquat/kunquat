# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2010
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from PyQt4 import QtGui, QtCore

import trigger


class TypeValidator(QtGui.QValidator):

    def validate(self, field, pos):
        if field in trigger.type_desc:
            return QtGui.QValidator.Acceptable, pos
        return QtGui.QValidator.Intermediate, pos


class TypeEdit(QtGui.QLineEdit):

    def __init__(self, parent=None):
        super(TypeEdit, self).__init__(parent)
        self.setValidator(TypeValidator())

    def keyPressEvent(self, ev):
        super(TypeEdit, self).keyPressEvent(ev)
        if ev.key() not in (QtCore.Qt.Key_Escape, QtCore.Qt.Key_Return):
            ev.accept()

    def set_value(self, value):
        self.setText(value)

    def get_value(self):
        return trigger.TriggerType(self.text())


