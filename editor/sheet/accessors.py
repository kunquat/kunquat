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

from __future__ import print_function

from PyQt4 import QtGui, QtCore

import trigger


class FuncValidator(QtGui.QValidator):

    def __init__(self, func=lambda x: False):
        super(FuncValidator, self).__init__()
        self.isvalid = func

    def validate(self, field, pos):
        if self.isvalid(field):
            return QtGui.QValidator.Acceptable, pos
        return QtGui.QValidator.Intermediate, pos


class Edit(QtGui.QLineEdit):

    def __init__(self, parent=None):
        super(Edit, self).__init__(parent)
        self.setValidator(FuncValidator())

    def keyPressEvent(self, ev):
        super(Edit, self).keyPressEvent(ev)
        if ev.key() not in (QtCore.Qt.Key_Escape, QtCore.Qt.Key_Return):
            ev.accept()

    def set_validator_func(self, func):
        self.setValidator(FuncValidator(func))


class TypeEdit(Edit):

    def set_value(self, value):
        self.setText(value)

    def get_value(self):
        return trigger.TriggerType(self.text())


