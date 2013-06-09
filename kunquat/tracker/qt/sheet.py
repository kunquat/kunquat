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
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

import tstamp


COLUMN_COUNT = 64


DEFAULT_CONFIG = {
        'ruler': {
            'bg_colour'     : QColor(0x11, 0x22, 0x55),
            'fg_colour'     : QColor(0xaa, 0xcc, 0xff),
            'font'          : QFont(QFont().defaultFamily(), 9),
            'line_min_dist' : 3,
            'line_len_short': 2,
            'line_len_long' : 4,
            'num_min_dist'  : 48,
            },
        'header': {
            },
        'ruler_width'  : 40,
        'col_width'    : 128,
        'px_per_beat'  : 128,
        }


class Sheet(QAbstractScrollArea):

    def __init__(self, config={}):
        QAbstractScrollArea.__init__(self)

        # Widgets
        self.setViewport(SheetView())

        self._corner = QWidget()

        self._ruler = Ruler()
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
        self._px_per_beat = self._config['px_per_beat']

        # XXX: testing
        self._total_length = tstamp.Tstamp(16)
        self._total_height_px = int(float(self._total_length) * self._px_per_beat)

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
        self._header.setFixedHeight(header_height)

        self._header.set_config(self._config['header'])
        self._ruler.set_config(self._config['ruler'])
        self.viewport().set_config(self._config)

    def set_ui_model(self, ui_model):
        self._stat_manager = ui_model.get_stat_manager()
        #self._stat_manager.register_update(self.update_xxx)

    def paintEvent(self, ev):
        self.viewport().paintEvent(ev)

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
        self._ruler.set_px_offset(vvalue)


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
            header.set_config(self._config) # FIXME: bad idea
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


class Ruler(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self._px_offset = 0
        self._cache = RulerCache()

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_config(self, config):
        self._config = config
        self._cache.set_config(config)
        self.update()

    def set_px_offset(self, offset):
        changed = offset != self._px_offset
        self._px_offset = offset
        if changed:
            self.update()

    def paintEvent(self, ev):
        start = time.time()

        painter = QPainter(self)

        # Testing
        canvas_y = 0
        for (src_rect, pixmap) in self._cache.iter_pixmaps(
                self._px_offset, self.height()):
            dest_rect = QRect(0, canvas_y, self.width(), src_rect.height())
            painter.drawPixmap(dest_rect, pixmap, src_rect)
            canvas_y += src_rect.height()

        end = time.time()
        elapsed = end - start
        print('Ruler updated in {:.2f} ms'.format(elapsed * 1000))

    def resizeEvent(self, ev):
        self._cache.set_width(ev.size().width())


class RulerCache():

    PIXMAP_HEIGHT = 256

    def __init__(self):
        self._width = 0
        self._px_per_beat = DEFAULT_CONFIG['px_per_beat']
        self._pixmaps = {}

    def set_config(self, config):
        self._config = config

    def set_width(self, width):
        if width != self._width:
            self._pixmaps = {}
        self._width = width

    def set_px_per_beat(self, px_per_beat):
        if px_per_beat != self._px_per_beat:
            self._pixmaps = {}
        self._px_per_beat = px_per_beat

    def iter_pixmaps(self, start_px, height_px):
        assert start_px >= 0
        assert height_px >= 0

        stop_px = start_px + height_px

        # Get pixmap indices
        start_index = start_px // RulerCache.PIXMAP_HEIGHT
        stop_index = 1 + (start_px + height_px - 1) // RulerCache.PIXMAP_HEIGHT

        create_count = 0

        for i in xrange(start_index, stop_index):
            if i not in self._pixmaps:
                self._pixmaps[i] = self._create_pixmap(i)
                create_count += 1

            # Get rect to be used
            pixmap_start_px = i * RulerCache.PIXMAP_HEIGHT
            rect_start_abs = max(start_px, pixmap_start_px)
            rect_start = rect_start_abs - pixmap_start_px

            pixmap_stop_px = (i + 1) * RulerCache.PIXMAP_HEIGHT
            rect_stop_abs = min(stop_px, pixmap_stop_px)
            rect_stop = rect_stop_abs - pixmap_start_px
            rect_height = rect_stop - rect_start

            rect = QRect(0, rect_start, self._width, rect_height)

            yield (rect, self._pixmaps[i])

        if create_count > 0:
            print('{} ruler pixmap{} created'.format(
                create_count, 's' if create_count != 1 else ''))

    def _create_pixmap(self, index):
        pixmap = QPixmap(self._width, RulerCache.PIXMAP_HEIGHT)

        cfg = self._config

        painter = QPainter(pixmap)

        # Background
        painter.setBackground(cfg['bg_colour'])
        painter.eraseRect(QRect(0, 0, self._width - 1, RulerCache.PIXMAP_HEIGHT))
        painter.setPen(cfg['fg_colour'])
        painter.drawLine(
                QPoint(self._width - 1, 0),
                QPoint(self._width - 1, RulerCache.PIXMAP_HEIGHT - 1))

        # Start border
        if index == 0:
            painter.drawLine(QPoint(0, 0), QPoint(self._width - 2, 0))

        # Start and stop timestamps
        start_ts = tstamp.Tstamp(0, tstamp.BEAT *
                index * RulerCache.PIXMAP_HEIGHT // self._px_per_beat)
        stop_ts = tstamp.Tstamp(0, tstamp.BEAT *
                (index + 1) * RulerCache.PIXMAP_HEIGHT // self._px_per_beat)

        # Ruler lines
        beat_div_base = 2
        if cfg['line_min_dist'] <= self._px_per_beat:
            lines_per_beat = self._px_per_beat // cfg['line_min_dist']
            lines_per_beat = beat_div_base**math.floor(
                    math.log(lines_per_beat, beat_div_base))

            # First visible line in the first beat
            start_beat_frac = start_ts.rem / float(tstamp.BEAT)
            start_line_in_beat = int(math.ceil(start_beat_frac * lines_per_beat))

            # First non-visible line in the last beat
            stop_beat_frac = stop_ts.rem / float(tstamp.BEAT)
            stop_line_in_beat = int(math.ceil(stop_beat_frac * lines_per_beat))

            def normalise_marker_pos(pos):
                if pos[1] >= lines_per_beat:
                    assert pos[1] == lines_per_beat
                    pos[0] += 1
                    pos[1] = 0

            # Loop boundaries
            line_pos = [start_ts.beats, start_line_in_beat]
            normalise_marker_pos(line_pos)

            stop_pos = [stop_ts.beats, stop_line_in_beat]
            normalise_marker_pos(stop_pos)

            # Draw lines
            while line_pos < stop_pos:
                ts = tstamp.Tstamp(line_pos[0] +
                        line_pos[1] / float(lines_per_beat))
                y = float(ts - start_ts) * self._px_per_beat

                line_length = (cfg['line_len_long']
                        if line_pos[1] == 0
                        else cfg['line_len_short'])
                painter.drawLine(
                        QPoint(self._width - 1 - line_length, y),
                        QPoint(self._width - 1, y))

                # Next line
                line_pos[1] += 1
                normalise_marker_pos(line_pos)
        else:
            pass

        # Testing
        painter.setFont(self._config['font'])
        painter.drawText(QPoint(2, 12), str(index))

        return pixmap


class SheetView(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_config(self, config):
        self._config = config

    def paintEvent(self, ev):
        start = time.time()

        painter = QPainter(self)

        # Testing
        painter.setBackground(Qt.black)
        painter.eraseRect(QRect(0, 0, self.width(), self.height()))
        painter.setPen(Qt.white)
        painter.drawRect(0, 0, self.width() - 1, self.height() - 1)

        end = time.time()
        elapsed = end - start
        print('SheetView updated in {:.2f} ms'.format(elapsed * 1000))


