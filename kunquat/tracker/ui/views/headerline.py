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


class HeaderLine(QWidget):

    def __init__(self, text):
        super().__init__()

        header = QLabel(text)
        header.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Minimum)

        header_line = QFrame()
        header_line.setObjectName('Header_line')
        header_line.setFrameShape(QFrame.HLine)
        header_line.setFrameShadow(QFrame.Sunken)
        header_line.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.Maximum)

        h = QHBoxLayout()
        h.setContentsMargins(5, 0, 5, 0)
        h.setSpacing(10)
        h.addWidget(header)
        h.addWidget(header_line)

        self.setLayout(h)

    def update_style(self, style_mgr):
        margin = style_mgr.get_scaled_size_param('medium_padding')
        spacing = style_mgr.get_scaled_size_param('large_padding')

        self.layout().setContentsMargins(margin, 0, margin, 0)
        self.layout().setSpacing(spacing)


