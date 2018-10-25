# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2015-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from kunquat.tracker.ui.qt import *


class ConfirmDialog(QDialog):

    def __init__(self, ui_model):
        super().__init__()
        style_mgr = ui_model.get_style_manager()
        icon_bank = ui_model.get_icon_bank()

        warning_img_orig = QPixmap(icon_bank.get_icon_path('warning'))
        warning_img = warning_img_orig.scaledToWidth(
                style_mgr.get_scaled_size_param('dialog_icon_size'),
                Qt.SmoothTransformation)
        warning_label = QLabel()
        warning_label.setPixmap(warning_img)

        self._message = QLabel()
        self._message.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Preferred)

        h = QHBoxLayout()
        margin = style_mgr.get_scaled_size_param('large_padding')
        h.setContentsMargins(margin, margin, margin, margin)
        h.setSpacing(margin * 2)
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


