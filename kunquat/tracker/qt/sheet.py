# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import division, print_function

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import tstamp


COLUMN_COUNT = 64


DEFAULT_CONFIG = {
        'ruler': {
            },
        'header': {
            },
        'ruler_width'  : 40,
        'col_width'    : 128,
        'rems_per_px'  : tstamp.BEAT // 128,
        }


class Sheet(QAbstractScrollArea):

    def __init__(self, config={}):
        QAbstractScrollArea.__init__(self)

        # Widgets
        self.setViewport(SheetView())

        self._corner = QWidget()

        self._ruler = QLabel('ruler')
        self._header = SheetHeader()

        # Config
        self._config = None
        self._set_config(config)

        # Layout
        g = QGridLayout()
        g.setSpacing(0)
        g.setMargin(0)
        g.addWidget(self._corner, 0, 0)
        g.addWidget(self._ruler, 1, 0)
        g.addWidget(self._header, 0, 1)
        g.addWidget(self.viewport(), 1, 1)
        self.setLayout(g)

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        self._col_width = self._config['col_width']
        self._rems_per_px = self._config['rems_per_px']

        # XXX: testing
        self._total_length = tstamp.Tstamp(16)
        rems = (self._total_length.beats * tstamp.BEAT + self._total_length.rem)
        self._total_height_px = rems // self._rems_per_px

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

        for subcfg in ('ruler', 'header'):
            self._config[subcfg] = DEFAULT_CONFIG[subcfg].copy()
            if subcfg in config:
                self._config[subcfg].update(config[subcfg])

        header_height = self._header.minimumSizeHint().height()

        self.setViewportMargins(
                self._config['ruler_width'],
                header_height,
                0, 0)

        self._corner.setFixedSize(
                self._config['ruler_width'],
                header_height)

        self._header.set_config(self._config['header'])
        self.viewport().set_config(self._config)

    def set_ui_model(self, ui_model):
        self._stat_manager = ui_model.get_stat_manager()
        #self._stat_manager.register_update(self.update_xxx)

    def resizeEvent(self, ev):
        vp_height = self.viewport().height()
        vscrollbar = self.verticalScrollBar()
        vscrollbar.setPageStep(vp_height)
        vscrollbar.setRange(0, self._total_height_px - vp_height)

        vp_width = self.viewport().width()
        max_visible_cols = vp_width // self._col_width
        hscrollbar = self.horizontalScrollBar()
        hscrollbar.setPageStep(max_visible_cols)
        hscrollbar.setRange(0, COLUMN_COUNT - max_visible_cols)

    def scrollContentsBy(self, dx, dy):
        hvalue = self.horizontalScrollBar().value()
        vvalue = self.verticalScrollBar().value()

        self._header.set_first_column(hvalue)


class SheetHeader(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._col_width = DEFAULT_CONFIG['col_width']
        self._first_col = 0

        self._headers = []

    def set_config(self, config):
        self._config = config
        self._update_contents()

    def set_column_width(self, width):
        self._col_width = width
        self._update_contents()

    def set_first_column(self, num):
        self._first_col = num
        self._update_contents()

    def resizeEvent(self, ev):
        self._update_contents()

    def minimumSizeHint(self):
        w = self._headers[0] if self._headers else ColumnHeader()
        sh = w.minimumSizeHint()
        return QSize(self._col_width * 3, sh.height())

    def _clamp_position(self, max_visible_cols):
        self._first_col = min(
                self._first_col,
                COLUMN_COUNT - max_visible_cols + 1)

    def _resize_layout(self, max_visible_cols):
        visible_cols = max_visible_cols
        if self._first_col + max_visible_cols > COLUMN_COUNT:
            visible_cols -= 1

        for i in xrange(len(self._headers), visible_cols):
            header = ColumnHeader()
            header.setParent(self)
            header.show()
            self._headers.append(header)
        for i in xrange(visible_cols, len(self._headers)):
            h = self._headers.pop()
            h.hide()

    def _update_contents(self):
        max_visible_cols = self.width() // self._col_width + 1
        max_visible_cols = min(max_visible_cols, COLUMN_COUNT + 1)

        self._clamp_position(max_visible_cols)

        self._resize_layout(max_visible_cols)

        # Update headers
        for i, header in enumerate(self._headers):
            header.set_config(self._config)
            header.set_column(self._first_col + i)
            header.move(i * self._col_width, 0)
            header.setFixedWidth(self._col_width)


class ColumnHeader(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._label = QLabel()
        self._label.setAlignment(Qt.AlignCenter)

        h = QHBoxLayout()
        h.setMargin(0)
        h.setSpacing(0)
        h.addWidget(self._label)

        self.setLayout(h)

    def set_config(self, config):
        self._config = config

    def set_column(self, num):
        self._num = num
        self._label.setText('{}'.format(num))


class SheetView(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_config(self, config):
        self._config = config


