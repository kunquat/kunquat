# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from __future__ import print_function
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from config import *
from header import Header
from ruler import Ruler
import utils
from view import View
import kunquat.tracker.ui.model.tstamp as tstamp


class Sheet(QAbstractScrollArea):

    def __init__(self, config={}):
        QAbstractScrollArea.__init__(self)

        # Widgets
        self.setViewport(View())

        self._corner = QWidget()

        self._ruler = Ruler()
        self._header = Header()

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

        self.viewport().setFocusProxy(None)

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        self._col_width = self._config['col_width']
        self._px_per_beat = None

        QObject.connect(
                self.viewport(),
                SIGNAL('heightChanged()'),
                self._update_scrollbars)
        QObject.connect(
                self.viewport(),
                SIGNAL('followCursor(int, int)'),
                self._follow_cursor)

    def set_ui_model(self, ui_model):
        self._ruler.set_ui_model(ui_model)
        self.viewport().set_ui_model(ui_model)

    def unregister_updaters(self):
        self._ruler.unregister_updaters()
        self.viewport().unregister_updaters()

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

        for subcfg in ('ruler', 'header', 'trigger', 'edit_cursor'):
            self._config[subcfg] = DEFAULT_CONFIG[subcfg].copy()
            if subcfg in config:
                self._config[subcfg].update(config[subcfg])

        self._header.set_config(self._config['header'])
        self._ruler.set_config(self._config['ruler'])

        header_height = self._header.minimumSizeHint().height()
        ruler_width = self._ruler.sizeHint().width()

        self.setViewportMargins(
                ruler_width,
                header_height,
                0, 0)

        self._corner.setFixedSize(
                ruler_width,
                header_height)
        self._header.setFixedHeight(header_height)
        self._ruler.setFixedWidth(ruler_width)

        fm = QFontMetrics(self._config['font'], self)
        self._config['font_metrics'] = fm
        self._config['tr_height'] = fm.tightBoundingRect('Ag').height() + 1

        self.viewport().set_config(self._config)

        # Default zoom level
        self._px_per_beat = self._config['trs_per_beat'] * self._config['tr_height']
        self._ruler.set_px_per_beat(self._px_per_beat)
        self.viewport().set_px_per_beat(self._px_per_beat)

    def _update_scrollbars(self):
        self._total_height_px = (
                self.viewport().get_total_height() + self._config['tr_height'])

        vp_height = self.viewport().height()
        vscrollbar = self.verticalScrollBar()
        vscrollbar.setPageStep(vp_height)
        vscrollbar.setRange(0, self._total_height_px - vp_height)

        vp_width = self.viewport().width()
        max_visible_cols = vp_width // self._col_width
        hscrollbar = self.horizontalScrollBar()
        hscrollbar.setPageStep(max_visible_cols)
        hscrollbar.setRange(0, COLUMN_COUNT - max_visible_cols)

    def _follow_cursor(self, new_y_offset, new_first_col):
        vscrollbar = self.verticalScrollBar()
        hscrollbar = self.horizontalScrollBar()
        old_y_offset = vscrollbar.value()
        old_first_col = hscrollbar.value()

        self.horizontalScrollBar().setValue(new_first_col)
        self.verticalScrollBar().setValue(new_y_offset)

        # Position not changed, so just update our viewport, TODO: kludgy
        if old_y_offset == vscrollbar.value() and old_first_col == hscrollbar.value():
            self.viewport().update()

    def paintEvent(self, ev):
        self.viewport().paintEvent(ev)

    def resizeEvent(self, ev):
        self._update_scrollbars()

    def scrollContentsBy(self, dx, dy):
        hvalue = self.horizontalScrollBar().value()
        vvalue = self.verticalScrollBar().value()

        self._header.set_first_column(hvalue)
        self._ruler.set_px_offset(vvalue)

        vp = self.viewport()
        vp.set_first_column(hvalue)
        vp.set_px_offset(vvalue)


