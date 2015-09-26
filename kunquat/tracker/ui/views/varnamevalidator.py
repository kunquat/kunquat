# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import string

from PyQt4.QtCore import *
from PyQt4.QtGui import *


class VarNameValidator(QValidator):

    def __init__(self, used_names):
        QValidator.__init__(self)
        self._used_names = used_names

    def validate(self, contents, pos):
        in_str = unicode(contents)
        if not in_str:
            return (QValidator.Intermediate, pos)

        allowed_init_chars = '_' + string.ascii_lowercase
        allowed_chars = allowed_init_chars + string.digits

        if in_str[0] not in allowed_init_chars:
            return (QValidator.Invalid, pos)

        if all(ch in allowed_chars for ch in in_str):
            if in_str not in self._used_names:
                return (QValidator.Acceptable, pos)
            else:
                return (QValidator.Intermediate, pos)

        return (QValidator.Invalid, pos)


