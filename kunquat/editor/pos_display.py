#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2012
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function

from PyQt4 import QtCore, QtGui

import timestamp as ts


class PosDisplay(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._project.set_callback('Asubsong', self._update_subsong)
        self._project.set_callback('Asection', self._update_section)
        self._project.set_callback('Apattern', self._update_pattern)
        self._project.set_callback('Arow', self._update_row)
        self._colours = {
                'bg': QtGui.QColor(0, 0, 0),
                'fg': QtGui.QColor(0x88, 0xdd, 0x88),
                }
        self._fonts = {
                'nums': QtGui.QFont('Decorative', 14, QtGui.QFont.Bold),
                }
        self._metrics = QtGui.QFontMetrics(self._fonts['nums'])
        self._margin = 10, 5
        self.setSizePolicy(QtGui.QSizePolicy.Fixed,
                           QtGui.QSizePolicy.Fixed)
        self._subsong = 0
        self._section = 0
        self._pattern = 0
        self._row = 0.0

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setBackground(self._colours['bg'])
        paint.eraseRect(ev.rect())
        paint.setPen(self._colours['fg'])
        paint.setFont(self._fonts['nums'])
        s = '{0}/{1}({2})/{3:.1f}'.format(self._subsong, self._section,
                                         self._pattern, self._row)
        rect = QtCore.QRectF(self._margin[0], self._margin[1],
                             self._metrics.width(s), self._metrics.height())
        paint.drawText(rect, s, QtGui.QTextOption(QtCore.Qt.AlignCenter))
        paint.end()

    def resizeEvent(self, ev):
        self.update()

    def sizeHint(self):
        return QtCore.QSize(self._metrics.width('000/000(1000)/00.0') +
                                    self._margin[0] * 2,
                            self._metrics.height() +
                                    self._margin[1] * 2)

    def _update_subsong(self, ch, event):
        self._subsong = event[1]

    def _update_section(self, ch, event):
        self._section = event[1]

    def _update_pattern(self, ch, event):
        self._pattern = event[1]

    def _update_row(self, ch, event):
        self._row = float(ts.Timestamp(event[1]))
        self.update()


