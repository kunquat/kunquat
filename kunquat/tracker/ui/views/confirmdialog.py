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


class ConfirmDialog(QDialog):

    def __init__(self, icon_bank):
        super().__init__()

        warning_img_path = icon_bank.get_icon_path('warning')
        warning_label = QLabel()
        warning_label.setPixmap(QPixmap(warning_img_path))

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        h.setMargin(8)
        h.setSpacing(16)
        h.addWidget(warning_label)
        h.addWidget(self._message)

        self._button_layout = QHBoxLayout()

        v = QVBoxLayout()
        v.addLayout(h)
        v.addLayout(self._button_layout)

        self.setLayout(v)

    # Protected interface

    def _set_message(self, msg):
        self._message.setText(msg)

    def _get_button_layout(self):
        return self._button_layout


