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

import string

from PyQt4.QtCore import *
from PyQt4.QtGui import *


def get_var_name_validation_status(text):
    assert isinstance(text, str)

    if not text:
        return QValidator.Intermediate

    allowed_init_chars = '_' + string.ascii_lowercase
    allowed_chars = allowed_init_chars + string.digits

    if text[0] not in allowed_init_chars:
        return QValidator.Invalid

    if all(ch in allowed_chars for ch in text):
        return QValidator.Acceptable

    return QValidator.Invalid


class VarNameValidator(QValidator):

    def __init__(self, used_names):
        QValidator.__init__(self)
        self._used_names = used_names

    def validate(self, contents, pos):
        in_str = str(contents)
        status = get_var_name_validation_status(in_str)
        if (status == QValidator.Acceptable) and (in_str in self._used_names):
            return (QValidator.Intermediate, contents, pos)
        return (status, contents, pos)


class MaybeVarNameValidator(QValidator):

    def __init__(self, used_names=set()):
        QValidator.__init__(self)
        self._used_names = used_names

    def validate(self, contents, pos):
        in_str = str(contents)
        if not in_str:
            return (QValidator.Acceptable, contents, pos)

        status = get_var_name_validation_status(in_str)
        if (status == QValidator.Acceptable) and (in_str in self._used_names):
            return (QValidator.Intermediate, contents, pos)
        return (status, contents, pos)


