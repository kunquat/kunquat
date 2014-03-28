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


class LongScrollBar(QScrollBar):

    def __init__(self):
        QScrollBar.__init__(self)
        self._range_factor = 1
        self._actual_min = self.minimum()
        self._actual_max = self.maximum()
        self._actual_value = self.value()

    def set_actual_range(self, actual_min, actual_max):
        self._actual_min = actual_min
        self._actual_max = max(actual_min, actual_max)

        self._range_factor = 1
        if self._actual_max > 0:
            digits_estimate = int(math.log(self._actual_max, 2))
            if digits_estimate >= 30:
                self._range_factor = 2**(digits_estimate - 29)

        scaled_min = self._actual_min / self._range_factor
        scaled_max = self._actual_max / self._range_factor
        QScrollBar.setRange(self, scaled_min, scaled_max)
        self.set_actual_value(self._actual_value)

    def set_actual_value(self, actual_value):
        self._actual_value = min(max(self._actual_min, actual_value), self._actual_max)
        QScrollBar.setValue(self, self._actual_value / self._range_factor)

    def get_actual_value(self):
        return self._actual_value

    def sliderChange(self, change):
        QScrollBar.sliderChange(self, change)
        if change == QAbstractSlider.SliderValueChange:
            self._update_actual_value(self.value())

    def _update_actual_value(self, scaled_value):
        self._actual_value = min(max(
            self._actual_min, scaled_value * self._range_factor), self._actual_max)


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

        self.setVerticalScrollBar(LongScrollBar())
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        self._col_width = self._config['col_width']

        QObject.connect(
                self.viewport(),
                SIGNAL('heightChanged()'),
                self._update_scrollbars)
        QObject.connect(
                self.viewport(),
                SIGNAL('followCursor(QString, int)'),
                self._follow_cursor)
        QObject.connect(
                self.viewport(),
                SIGNAL('zoom(int)'),
                self._zoom)

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
        px_per_beat = self._config['trs_per_beat'] * self._config['tr_height']

        self._zoom_levels = self._get_zoom_levels(1, px_per_beat, tstamp.BEAT)
        self._cur_zoom_index = self._zoom_levels.index(px_per_beat)
        self._default_zoom_index = self._cur_zoom_index

        self._update_px_per_beat(self._zoom_levels[self._cur_zoom_index])

    def _get_zoom_levels(self, min_val, default_val, max_val):
        zoom_levels = [default_val]

        # Fill zoom out levels until minimum
        prev_val = zoom_levels[-1]
        next_val = prev_val / self._config['zoom_factor']
        while int(next_val) > min_val:
            actual_val = int(next_val)
            assert actual_val < prev_val
            zoom_levels.append(actual_val)
            prev_val = actual_val
            next_val = prev_val / self._config['zoom_factor']
        zoom_levels.append(min_val)
        zoom_levels = list(reversed(zoom_levels))

        # Fill zoom in levels until maximum
        prev_val = zoom_levels[-1]
        next_val = prev_val * self._config['zoom_factor']
        while math.ceil(next_val) < tstamp.BEAT:
            actual_val = int(math.ceil(next_val))
            assert actual_val > prev_val
            zoom_levels.append(actual_val)
            prev_val = actual_val
            next_val = prev_val * self._config['zoom_factor']
        zoom_levels.append(tstamp.BEAT)

        return zoom_levels

    def _update_px_per_beat(self, px_per_beat):
        self._ruler.set_px_per_beat(px_per_beat)
        self.viewport().set_px_per_beat(px_per_beat)

    def _update_scrollbars(self):
        self._total_height_px = (
                self.viewport().get_total_height() + self._config['tr_height'])

        vp_height = self.viewport().height()
        vscrollbar = self.verticalScrollBar()
        vscrollbar.setPageStep(vp_height)
        vscrollbar.set_actual_range(0, self._total_height_px - vp_height)

        vp_width = self.viewport().width()
        max_visible_cols = vp_width // self._col_width
        hscrollbar = self.horizontalScrollBar()
        hscrollbar.setPageStep(max_visible_cols)
        hscrollbar.setRange(0, COLUMN_COUNT - max_visible_cols)

    def _follow_cursor(self, new_y_offset_str, new_first_col):
        new_y_offset = long(new_y_offset_str)
        vscrollbar = self.verticalScrollBar()
        hscrollbar = self.horizontalScrollBar()
        old_y_offset = vscrollbar.get_actual_value()
        old_scaled_y_offset = vscrollbar.value()
        old_first_col = hscrollbar.value()

        vscrollbar.set_actual_value(new_y_offset)
        hscrollbar.setValue(new_first_col)

        if (old_scaled_y_offset == vscrollbar.value() and
                old_first_col == hscrollbar.value()):
            if old_y_offset != vscrollbar.get_actual_value():
                # Position changed slightly, but the QScrollBar doesn't know this
                self.scrollContentsBy(0, 0)
            else:
                # Position not changed, so just update our viewport
                self.viewport().update()

    def _zoom(self, update):
        assert self._zoom_levels
        if update == 0:
            self._cur_zoom_index = self._default_zoom_index
        else:
            new_index = self._cur_zoom_index + update
            self._cur_zoom_index = min(max(0, new_index), len(self._zoom_levels) - 1)

        self._update_px_per_beat(self._zoom_levels[self._cur_zoom_index])

    def paintEvent(self, ev):
        self.viewport().paintEvent(ev)

    def resizeEvent(self, ev):
        self._update_scrollbars()

    def scrollContentsBy(self, dx, dy):
        hvalue = self.horizontalScrollBar().value()
        vvalue = self.verticalScrollBar().get_actual_value()

        self._header.set_first_column(hvalue)
        self._ruler.set_px_offset(vvalue)

        vp = self.viewport()
        vp.set_first_column(hvalue)
        vp.set_px_offset(vvalue)


