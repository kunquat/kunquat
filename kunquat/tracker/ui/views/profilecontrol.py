# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import cProfile
import pstats
from io import StringIO

from PySide.QtCore import *
from PySide.QtGui import *


class ProfileControl(QDialog):

    SORTS = {
            'Sort by cumulative time': 'cumtime',
            'Sort by total time': 'tottime',
            'Sort by number of calls': 'ncalls',
            }

    # Work around compatibility issues with older Pythons
    SORT_HACKS = {
            'cumtime': 2,
            'tottime': 1,
            'ncalls': 0,
            }

    def __init__(self):
        super().__init__()
        self.resize(1360, 720)
        self._stats = None
        self._stats_output = None
        self._sort_key = 'cumtime'
        self._running = False
        self._profiler = cProfile.Profile()

        self._sort_selector = QComboBox()
        for sort in sorted(self.SORTS.keys()):
            self._sort_selector.addItem(sort)
        QObject.connect(
                self._sort_selector,
                SIGNAL('currentIndexChanged(QString)'),
                self._update_sort)

        self._toggle = QPushButton()
        self._toggle.setText('Start profiling')
        QObject.connect(
                self._toggle, SIGNAL('clicked()'), self._toggle_profiling)

        self._details = QTextEdit()
        self._details.setAcceptRichText(False)
        self._details.setReadOnly(True)
        self._details.setFontFamily('monospace')

        v = QVBoxLayout()
        h = QHBoxLayout()
        h.addWidget(self._sort_selector)
        h.addWidget(self._toggle)
        v.addItem(h)
        v.addWidget(self._details)
        self.setLayout(v)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

    def _set_profile_stats(self, profile):
        self._stats_output = StringIO()
        self._stats = pstats.Stats(profile, stream=self._stats_output)
        self._stats.sort_stats(self.SORT_HACKS[self._sort_key])
        self._stats.print_stats()
        data = self._stats_output.getvalue()
        self._details.setPlainText(data)

    def _update_sort(self, key):
        self._sort_key = self.SORTS[str(key)]
        if self._stats:
            self._stats_output.seek(0)
            self._stats_output.truncate()
            self._stats.sort_stats(self.SORT_HACKS[self._sort_key])
            self._stats.print_stats()
            data = self._stats_output.getvalue()
            self._details.setPlainText(data)

    def _toggle_profiling(self):
        if not self._running:
            self._toggle.setText('Stop profiling')
            self._profiler.enable()
            self._running = True
        else:
            self._toggle.setText('Start profiling')
            self._profiler.disable()
            self._set_profile_stats(self._profiler)
            self._profiler.clear()
            self._running = False


