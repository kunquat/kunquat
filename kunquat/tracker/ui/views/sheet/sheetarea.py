# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2013-2019
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math
import time

from kunquat.tracker.ui.qt import *

from kunquat.kunquat.limits import *
import kunquat.tracker.ui.model.tstamp as tstamp
from kunquat.tracker.ui.views.updater import Updater
from .config import *
from .header import Header
from .ruler import Ruler
from . import utils
from .view import View


class LongScrollBar(QScrollBar):

    def __init__(self):
        super().__init__()
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
        super().setRange(scaled_min, scaled_max)
        self.set_actual_value(self._actual_value)

    def set_actual_value(self, actual_value):
        self._actual_value = min(max(self._actual_min, actual_value), self._actual_max)
        super().setValue(self._actual_value / self._range_factor)

    def get_actual_value(self):
        return self._actual_value

    def sliderChange(self, change):
        super().sliderChange(change)
        if change == QAbstractSlider.SliderValueChange:
            self._update_actual_value(self.value())

    def _update_actual_value(self, scaled_value):
        self._actual_value = min(max(
            self._actual_min, scaled_value * self._range_factor), self._actual_max)


class Corner(QWidget):

    def __init__(self):
        super().__init__()

        self._bg_colour = QColor(0, 0, 0)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_config(self, config):
        self._bg_colour = config['bg_colour']
        self._disabled_colour = config['disabled_colour']

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setBackground(self._bg_colour)
        painter.eraseRect(event.rect())

        if not self.isEnabled():
            painter.fillRect(0, 0, self.width(), self.height(), self._disabled_colour)


class SheetArea(QAbstractScrollArea, Updater):

    def __init__(self):
        super().__init__()
        self.setFocusPolicy(Qt.NoFocus)

        # Widgets
        self.setViewport(View())

        self._corner = Corner()

        self._ruler = Ruler()
        self._header = Header()

        # Config
        self._config = {}

        # Layout
        g = QGridLayout()
        g.setSpacing(0)
        g.setContentsMargins(0, 0, 0, 0)
        g.addWidget(self._corner, 0, 0)
        g.addWidget(self._ruler, 1, 0)
        g.addWidget(self._header, 0, 1)
        g.addWidget(self.viewport(), 1, 1)
        self.setLayout(g)

        self.viewport().setFocusProxy(None)

        self.setVerticalScrollBar(LongScrollBar())
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)

        self.viewport().heightChanged.connect(self._update_scrollbars)
        self.viewport().followCursor.connect(self._follow_cursor)
        self.viewport().followPlaybackColumn.connect(self._follow_playback_column)

    def _on_setup(self):
        self.register_action('signal_sheet_zoom', self._update_zoom)
        self.register_action('signal_sheet_column_width', self._update_column_width)
        self.register_action('signal_style_changed', self._update_config)
        self.register_action('signal_controls', self._update_header)
        self.register_action('signal_ch_defaults', self._update_header)
        self.register_action('signal_channel_mute', self._update_header)

        self._sheet_mgr = self._ui_model.get_sheet_manager()

        # Child widgets
        self.add_to_updaters(self._ruler, self.viewport())

        self._header.set_ui_model(self._ui_model)

        self._update_config()

        self._updater.signal_update('signal_sheet_zoom')

    def _update_config(self):
        style_mgr = self._ui_model.get_style_manager()
        config = get_config_with_custom_style(style_mgr)
        self._set_config(config)

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

        fm = QFontMetrics(self._config['font'], self)
        self._config['font_metrics'] = fm
        self._config['tr_height'] = (fm.tightBoundingRect('A').height() +
                self._config['trigger']['padding_y'] * 2)

        # Set zoom levels
        px_per_beat = self._config['trs_per_beat'] * self._config['tr_height']
        self._zoom_levels = utils.get_zoom_levels(
                1, px_per_beat, tstamp.BEAT, self._config['zoom_factor'])
        self._default_zoom_index = self._zoom_levels.index(px_per_beat)
        self._sheet_mgr.set_zoom_range(
                -self._default_zoom_index,
                len(self._zoom_levels) - self._default_zoom_index - 1)

        # Set column widths
        fm = self._config['font_metrics']
        em_px = int(math.ceil(fm.tightBoundingRect('m').width()))
        em_range = list(range(3, 41))
        self._col_width_levels = [em_px * width for width in em_range]
        self._default_col_width_index = em_range.index(self._config['col_width'])
        self._sheet_mgr.set_column_width_range(
                -self._default_col_width_index,
                len(self._col_width_levels) - self._default_col_width_index - 1)

        subcfgs = ('ruler', 'header', 'trigger', 'edit_cursor', 'area_selection', 'grid')

        for subcfg in subcfgs:
            self._config[subcfg] = DEFAULT_CONFIG[subcfg].copy()
            if subcfg in config:
                self._config[subcfg].update(config[subcfg])

        self._corner.set_config(self._config)
        self._header.set_config(self._config)
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

        self.viewport().set_config(self._config)
        self._header.set_total_width(self.viewport().width())

        self._update_zoom()
        self._update_column_width()

    def _set_px_per_beat(self, px_per_beat):
        self._ruler.set_px_per_beat(px_per_beat)
        self.viewport().set_px_per_beat(px_per_beat)

    def _set_column_width(self, width):
        self._header.set_column_width(width)
        self.viewport().set_column_width(width)

    def _update_scrollbars(self):
        if not self._ui_model:
            return

        tr_height = self._config['tr_height']
        self._total_height_px = self.viewport().get_total_height() + tr_height

        vp_height = self.viewport().height()
        vscrollbar = self.verticalScrollBar()
        vscrollbar.setSingleStep(tr_height)
        vscrollbar.setPageStep(vp_height)
        vscrollbar.set_actual_range(0, self._total_height_px - vp_height)

        vp_width = self.viewport().width()
        cur_col_width_index = (
                self._sheet_mgr.get_column_width() + self._default_col_width_index)
        max_visible_cols = vp_width // self._col_width_levels[cur_col_width_index]
        hscrollbar = self.horizontalScrollBar()
        hscrollbar.setPageStep(max_visible_cols)
        hscrollbar.setRange(0, COLUMNS_MAX - max_visible_cols)

    def _follow_cursor(self, new_y_offset_str, new_first_col):
        new_y_offset = int(new_y_offset_str)
        vscrollbar = self.verticalScrollBar()
        hscrollbar = self.horizontalScrollBar()
        old_y_offset = vscrollbar.get_actual_value()
        old_scaled_y_offset = vscrollbar.value()
        old_first_col = hscrollbar.value()

        self._update_scrollbars()
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

    def _follow_playback_column(self, new_first_col):
        hscrollbar = self.horizontalScrollBar()
        old_first_col = hscrollbar.value()

        self._update_scrollbars()
        hscrollbar.setValue(new_first_col)
        self.viewport().update()

    def _update_zoom(self):
        zoom_level = self._sheet_mgr.get_zoom()
        cur_zoom_index = zoom_level + self._default_zoom_index
        self._set_px_per_beat(self._zoom_levels[cur_zoom_index])

    def _update_column_width(self):
        column_width_level = self._sheet_mgr.get_column_width()
        cur_col_width_index = column_width_level + self._default_col_width_index
        self._set_column_width(self._col_width_levels[cur_col_width_index])

    def _update_header(self):
        self._header.update_header_aus()

    def paintEvent(self, ev):
        self.viewport().paintEvent(ev)

    def resizeEvent(self, ev):
        self._update_scrollbars()
        self.viewport().resizeEvent(ev)
        self._header.set_total_width(self.viewport().width())

    def scrollContentsBy(self, dx, dy):
        hvalue = self.horizontalScrollBar().value()
        vvalue = self.verticalScrollBar().get_actual_value()

        self._header.set_first_column(hvalue)
        self._ruler.set_px_offset(vvalue)

        vp = self.viewport()
        vp.set_first_column(hvalue)
        vp.set_px_offset(vvalue)

    def mousePressEvent(self, event):
        self.viewport().mousePressEvent(event)

    def mouseMoveEvent(self, event):
        self.viewport().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        self.viewport().mouseReleaseEvent(event)

    def changeEvent(self, event):
        if event.type() == QEvent.EnabledChange:
            self._corner.update()
            self._header.update()
            self._ruler.update()
            self.viewport().update()


