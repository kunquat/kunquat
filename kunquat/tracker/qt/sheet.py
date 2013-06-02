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
                'width': 40,
            },
        'header': {
                'height': 20,
            },
        'col_width' : 128,
        'rem_per_px': tstamp.BEAT // 128,
        }


class Sheet(QAbstractScrollArea):

    def __init__(self, config={}):
        QAbstractScrollArea.__init__(self)

        # Widgets
        self.setViewport(SheetView())

        self._corner = QWidget()

        self._ruler = QLabel('ruler')
        self._header = QLabel('header')

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
        self._rem_per_px = self._config['rem_per_px']

        # XXX: testing
        vscrollbar = self.verticalScrollBar()
        vscrollbar.setRange(0, 2000)

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

        for subcfg in ('ruler', 'header'):
            self._config[subcfg] = DEFAULT_CONFIG[subcfg].copy()
            if subcfg in config:
                self._config[subcfg].update(config[subcfg])

        self.setViewportMargins(
                self._config['ruler']['width'],
                self._config['header']['height'],
                0, 0)

        self._corner.setFixedSize(
                self._config['ruler']['width'],
                self._config['header']['height'])

        self.viewport().set_config(self._config)

    def set_ui_model(self, ui_model):
        self._stat_manager = ui_model.get_stat_manager()
        #self._stat_manager.register_update(self.update_xxx)

    def resizeEvent(self, ev):
        vp_width = self.viewport().width()
        max_visible_cols = vp_width // self._col_width
        hscrollbar = self.horizontalScrollBar()
        hscrollbar.setPageStep(max_visible_cols)
        hscrollbar.setRange(0, COLUMN_COUNT - max_visible_cols)

    def scrollContentsBy(self, dx, dy):
        hvalue = self.horizontalScrollBar().value()
        vvalue = self.verticalScrollBar().value()


class SheetView(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_config(self, config):
        self._config = config


