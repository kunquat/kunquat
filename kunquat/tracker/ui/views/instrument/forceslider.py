# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
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


class ForceSlider(QWidget):

    SCALE = float(10)

    forceChanged = pyqtSignal(float, name='forceChanged')

    def __init__(self):
        QWidget.__init__(self)

        min_val = -64.0
        max_val = 18.0

        self._slider = QSlider()
        self._slider.setOrientation(Qt.Horizontal)
        self._slider.setMinimum(int(min_val * self.SCALE))
        self._slider.setMaximum(int(max_val * self.SCALE))

        self._value = QLabel()
        fm = QFontMetrics(QFont())
        val_fmt = '{:.1f}'
        width = max(fm.width(val_fmt.format(val)) for val in (min_val, max_val))
        self._value.setFixedWidth(width)

        h = QHBoxLayout()
        h.addWidget(self._slider)
        h.addWidget(self._value)
        self.setLayout(h)

        QObject.connect(self._slider, SIGNAL('valueChanged(int)'), self._force_changed)

        self.set_force(0)

    def set_force(self, force):
        old_block = self._slider.blockSignals(True)
        int_val = int(round(force * self.SCALE))
        self._slider.setValue(int_val)
        self._slider.blockSignals(old_block)

        self._value.setText('{:.1f}'.format(force))

    def _force_changed(self, int_val):
        force = int_val / self.SCALE
        QObject.emit(self, SIGNAL('forceChanged(float)'), force)


