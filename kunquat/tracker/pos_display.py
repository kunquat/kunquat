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
from collections import defaultdict

from PyQt4 import QtCore, QtGui

import timestamp as ts


PLAY = u'▶'
PLAY_INF = u'∞'
STOP = u'■'

NO_VAL = u'–'
SEP_SUB = '/'
PAT_BEG = '('
PAT_END = ')'
SEP_SEC = '/'


class PosDisplay(QtGui.QWidget):

    def __init__(self, project, parent=None):
        QtGui.QWidget.__init__(self, parent)
        self._project = project
        self._colours = {
                'bg': QtGui.QColor(0, 0, 0),
                'fg': QtGui.QColor(0x88, 0xdd, 0x88),
                PLAY: QtGui.QColor(0x88, 0xdd, 0x88),
                PLAY_INF: QtGui.QColor(0xdd, 0xcc, 0x88),
                STOP: QtGui.QColor(0x66, 0x66, 0x66),
                }
        fonts = [
                    ('large', QtGui.QFont('Decorative', 16, QtGui.QFont.Bold)),
                    ('def', QtGui.QFont('Decorative', 14, QtGui.QFont.Bold)),
                    ('small', QtGui.QFont('Decorative', 14, QtGui.QFont.Bold)),
                    ('tiny', QtGui.QFont('Decorative', 14, QtGui.QFont.Bold)),
                    ('etiny', QtGui.QFont('Decorative', 14, QtGui.QFont.Bold)),
                ]
        self._fonts = dict(fonts)
        self._fonts['small'].setStretch(QtGui.QFont.SemiCondensed)
        self._fonts['tiny'].setStretch(QtGui.QFont.Condensed)
        self._fonts['etiny'].setStretch(QtGui.QFont.ExtraCondensed)
        self._font_names = [n[0] for n in fonts[1:]]
        self._metrics = dict([(i[0], QtGui.QFontMetrics(i[1]))
                              for i in self._fonts.iteritems()])
        self._margin = 10, 5
        self._padding = 0
        self._widths = [
                    self._metrics['def'].width('0'),
                    self._metrics['small'].width(SEP_SUB),
                    self._metrics['def'].width('00'),
                    self._metrics['small'].width(PAT_BEG),
                    self._metrics['def'].width('00'),
                    self._metrics['small'].width(PAT_END),
                    self._metrics['small'].width(SEP_SEC),
                    self._metrics['def'].width('00.0'),
                ]
        self._status_sep = 5
        self._status_space = self._metrics['def'].width(STOP)
        offset = self._margin[0] + self._status_space + self._status_sep
        self._offsets = []
        for w in self._widths:
            self._offsets.extend([offset])
            offset += w + self._padding
        self.setSizePolicy(QtGui.QSizePolicy.Fixed,
                           QtGui.QSizePolicy.Fixed)
        self._subsong = NO_VAL
        self._section = NO_VAL
        self._pattern = NO_VAL
        self._row = 0.0
        self._upcoming = defaultdict(lambda: NO_VAL)
        self.set_stop()

    def init(self):
        self._project.set_callback('Asubsong', self._update_loc)
        self._project.set_callback('Asystem', self._update_loc)
        self._project.set_callback('Apattern', self._update_loc)
        self._project.set_callback('Arow', self._update_loc)

    def set_play(self, infinite=False):
        self._play_mode = PLAY_INF if infinite else PLAY

    def set_stop(self):
        self._play_mode = STOP
        self.update()

    def paintEvent(self, ev):
        paint = QtGui.QPainter()
        paint.begin(self)
        paint.setBackground(self._colours['bg'])
        paint.eraseRect(ev.rect())
        paint.setPen(self._colours[self._play_mode])
        if self._play_mode == PLAY_INF:
            rect = QtCore.QRectF(self._margin[0] - 3, self._margin[1],
                                 self._status_space + 6,
                                 self._metrics['def'].height())
            self.setFont(self._fonts['large'])
        else:
            rect = QtCore.QRectF(self._margin[0], self._margin[1] - 1,
                                 self._status_space,
                                 self._metrics['def'].height())
            self.setFont(self._fonts['def'])
        paint.drawText(rect, self._play_mode,
                       QtGui.QTextOption(QtCore.Qt.AlignCenter))
        paint.setPen(self._colours['fg'])
        if self._play_mode == STOP:
            elements = [ NO_VAL, SEP_SUB, NO_VAL, PAT_BEG, NO_VAL, PAT_END,
                         SEP_SEC, NO_VAL ]
        else:
            elements = [
                        unicode(self._subsong),
                        SEP_SUB,
                        unicode(self._section),
                        PAT_BEG,
                        unicode(self._pattern),
                        PAT_END,
                        SEP_SEC,
                        '{0:.1f}'.format(self._row),
                    ]
        for i, el in enumerate(elements):
            for name in self._font_names:
                font_name = name
                if self._metrics[font_name].width(el) <= self._widths[i]:
                    break
            paint.setFont(self._fonts[font_name])
            rect = QtCore.QRectF(self._offsets[i], self._margin[1],
                                 self._widths[i],
                                 self._metrics[font_name].height())
            paint.drawText(rect, el, QtGui.QTextOption(QtCore.Qt.AlignCenter))
        paint.end()

    def resizeEvent(self, ev):
        self.update()

    def sizeHint(self):
        return QtCore.QSize(2 * self._margin[0] + self._status_space +
                                    self._status_sep + sum(self._widths) +
                                    (len(self._widths) - 1) * self._padding,
                            2 * self._margin[1] +
                                    self._metrics['def'].height())

    def _update_loc(self, ch, event):
        if self._play_mode == STOP:
            return
        if event[0] == 'Arow':
            self._upcoming[event[0]] = float(ts.Timestamp(event[1]))
            if self._subsong == self._upcoming['Asong'] and \
                    self._section == self._upcoming['Asystem'] and \
                    self._pattern == self._upcoming['Apattern'] and \
                    self._row == self._upcoming['Arow']:
                self._upcoming = defaultdict(lambda: NO_VAL)
                return
            self._subsong = self._upcoming['Asong']
            self._section = self._upcoming['Asystem']
            self._pattern = self._upcoming['Apattern']
            self._row = self._upcoming['Arow']
            self._upcoming = defaultdict(lambda: NO_VAL)
            self.update()
        else:
            self._upcoming[event[0]] = event[1]


