# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2016
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


class IntValidator(QValidator):

    def __init__(self):
        QValidator.__init__(self)

    def validate(self, contents, pos):
        in_str = str(contents)
        if not in_str:
            return (QValidator.Intermediate, pos)

        stripped = in_str.strip()
        if stripped in ('+', '-'):
            return (QValidator.Intermediate, pos)

        try:
            value = int(stripped)
        except ValueError:
            return (QValidator.Invalid, pos)

        return (QValidator.Acceptable, pos)


class FloatValidator(QValidator):

    def __init__(self):
        QValidator.__init__(self)

    def validate(self, contents, pos):
        in_str = str(contents)
        if not in_str:
            return (QValidator.Intermediate, pos)

        stripped = in_str.strip()
        if stripped in ('+', '-', '.', '+.', '-.'):
            return (QValidator.Intermediate, pos)

        try:
            value = float(in_str)
        except ValueError:
            return (QValidator.Invalid, pos)

        return (QValidator.Acceptable, pos)


