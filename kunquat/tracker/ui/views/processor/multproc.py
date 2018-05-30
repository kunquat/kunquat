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

from .processorupdater import ProcessorUpdater


class MultProc(QWidget, ProcessorUpdater):

    @staticmethod
    def get_name():
        return 'Multiplication'

    def __init__(self):
        super().__init__()

        v = QVBoxLayout()
        v.addWidget(QLabel('The Multiplier does not support additional parameters.'))
        v.addStretch(1)
        self.setLayout(v)

        self.setSizePolicy(QSizePolicy.MinimumExpanding, QSizePolicy.MinimumExpanding)


