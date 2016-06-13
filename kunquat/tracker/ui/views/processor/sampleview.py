# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import heapq
import math
import time

from PySide.QtCore import *
from PySide.QtGui import *


class SampleView(QWidget):

    def __init__(self):
        super().__init__()

        self._toolbar = SampleViewToolBar()
        self._area = SampleViewArea()

        v = QVBoxLayout()
        v.setContentsMargins(0, 0, 0, 0)
        v.setSpacing(0)
        v.addWidget(self._toolbar)
        v.addWidget(self._area)
        self.setLayout(v)

        QObject.connect(self._toolbar, SIGNAL('zoomIn()'), self._area.zoom_in)
        QObject.connect(self._toolbar, SIGNAL('zoomOut()'), self._area.zoom_out)

    def set_icon_bank(self, icon_bank):
        self._toolbar.set_icon_bank(icon_bank)

    def set_sample(self, length, get_sample_data):
        self._area.set_sample(length, get_sample_data)


class SampleViewToolBar(QToolBar):

    zoomIn = Signal(name='zoomIn')
    zoomOut = Signal(name='zoomOut')

    def __init__(self):
        super().__init__()

        self._zoom_in = QToolButton()
        self._zoom_in.setText('Zoom In')
        self._zoom_in.setToolTip(self._zoom_in.text())

        self._zoom_out = QToolButton()
        self._zoom_out.setText('Zoom Out')
        self._zoom_out.setToolTip(self._zoom_out.text())

        self.addWidget(self._zoom_in)
        self.addWidget(self._zoom_out)

        QObject.connect(self._zoom_in, SIGNAL('clicked()'), self._signal_zoom_in)
        QObject.connect(self._zoom_out, SIGNAL('clicked()'), self._signal_zoom_out)

    def set_icon_bank(self, icon_bank):
        self._zoom_in.setIcon(QIcon(icon_bank.get_icon_path('zoom_in')))
        self._zoom_out.setIcon(QIcon(icon_bank.get_icon_path('zoom_out')))

    def _signal_zoom_in(self):
        QObject.emit(self, SIGNAL('zoomIn()'))

    def _signal_zoom_out(self):
        QObject.emit(self, SIGNAL('zoomOut()'))


class SampleViewArea(QAbstractScrollArea):

    def __init__(self):
        super().__init__()

        self.setViewport(SampleViewCanvas())
        self.viewport().setFocusProxy(None)

        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)

    def set_sample(self, length, get_sample_data):
        self.viewport().set_sample(length, get_sample_data)
        self._update_scrollbars()

    def zoom_in(self):
        start, stop = self.viewport().get_range()
        width = stop - start
        shrink = width // 4
        shrink_l, shrink_r = shrink, shrink

        # Make sure that at least 8 frames are visible
        max_shrink_total = width - 8
        if shrink * 2 > max_shrink_total:
            shrink_l = max_shrink_total // 2
            shrink_r = max_shrink_total - shrink_l

        self.set_range(start + shrink_l, stop - shrink_r)

    def zoom_out(self):
        start, stop = self.viewport().get_range()
        length = self.viewport().get_length()
        width = stop - start
        expand = width // 2
        start -= expand
        stop += expand

        # Shift the expanded range if we went out of sample boundaries
        if start < 0:
            shift = -start
            start += shift
            stop += shift
        elif stop > length:
            shift = stop - length
            start -= shift
            stop -= shift

        self.set_range(start, stop)

    def set_range(self, start, stop):
        assert start < stop
        start = max(0, start)
        stop = min(self.viewport().get_length(), stop)
        self.viewport().set_range(start, stop)
        self._update_scrollbars()

    def _update_scrollbars(self):
        scrollbar = self.horizontalScrollBar()

        view = self.viewport()
        length = view.get_length()

        if length == 0:
            scrollbar.setRange(0, 0)
            return

        start, stop = view.get_range()
        width = stop - start
        scrollbar.setPageStep(width)
        scrollbar.setRange(0, length - width)
        scrollbar.setValue(start)

    def paintEvent(self, event):
        self.viewport().paintEvent(event)

    def scrollContentsBy(self, dx, dy):
        value = self.horizontalScrollBar().value()

        view = self.viewport()
        view.set_position(value)

    def mouseMoveEvent(self, event):
        self.viewport().mouseMoveEvent(event)

    def mousePressEvent(self, event):
        self.viewport().mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        self.viewport().mouseReleaseEvent(event)

    def resizeEvent(self, event):
        self.viewport().resizeEvent(event)


class SampleViewCanvas(QWidget):

    refresh = Signal(name='refresh')

    _REF_PIXMAP_WIDTH = 256

    def __init__(self):
        super().__init__()

        self._length = 0

        self._range = [0, 0]

        self._shape_worker = ShapeWorker()
        self._pixmap_caches = {}

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setMouseTracking(True)

        QObject.connect(self, SIGNAL('refresh()'), self._refresh)

    def set_sample(self, length, get_sample_data):
        self._length = length
        self.set_range(0, length)

        self._shape_worker.reset(self._length, get_sample_data)
        self._pixmap_caches = {}

        self.update()

    def get_length(self):
        return self._length

    def set_range(self, start, stop):
        self._range = [start, stop]
        self.update()

    def get_range(self):
        return self._range

    def set_position(self, pos):
        start, stop = self._range
        pos_delta = pos - start
        start += pos_delta
        stop += pos_delta
        assert start >= 0
        assert stop <= self._length
        self._range = [start, stop]
        self.update()

    def _get_ref_frames_per_px(self, frames_per_px):
        return 2**math.floor(math.log(frames_per_px, 2))

    def _get_visible_pixmaps(self):
        height = self.height()

        # Get frames per pixel
        start, stop = self._range
        range_width = stop - start
        frames_per_px = range_width / self.width()

        # Get range width covered by a single shape slice
        ref_fpp = self._get_ref_frames_per_px(frames_per_px)
        pixmap_range_width = int(ref_fpp * self._REF_PIXMAP_WIDTH)

        src_rect = QRect(0, 0, self._REF_PIXMAP_WIDTH, height)
        dest_rect_width = (ref_fpp / frames_per_px) * self._REF_PIXMAP_WIDTH

        # Get pixmap indices
        start, stop = self._range
        start_i = start // pixmap_range_width
        stop_i = 1 + stop // pixmap_range_width

        # Get our pixmap cache
        pc = self._pixmap_caches.get(ref_fpp, {})
        self._pixmap_caches[ref_fpp] = pc

        view_start = start / frames_per_px

        # Update worker
        self._shape_worker.update()

        refresh_required = False

        for i in range(start_i, stop_i):
            start_x = int(i * dest_rect_width - view_start)
            stop_x = int((i + 1) * dest_rect_width - view_start)

            dest_rect = QRect(start_x, 0, stop_x - start_x, height)
            if i not in pc:
                slice_start = i * pixmap_range_width
                slice_stop = (i + 1) * pixmap_range_width
                if slice_start >= stop:
                    break

                shape = self._shape_worker.request_shape(
                        ref_fpp, (slice_start, slice_stop))

                if shape:
                    pixmap = QPixmap(self._REF_PIXMAP_WIDTH, height)
                    pixmap.fill(QColor(0, 0, 0))

                    painter = QPainter(pixmap)
                    painter.setPen(QColor(0xff, 0xff, 0xff))
                    painter.setBrush(QColor(0xff, 0xff, 0xff))
                    painter.scale(1, height / 2)
                    painter.translate(0, 1)
                    shape.draw_shape(painter)

                    '''
                    painter = QPainter(pixmap)
                    painter.setPen(QColor(0xff, 0xff, 0xff))
                    test_rect = src_rect.adjusted(0, 0, -1, -1)
                    painter.drawRect(test_rect)

                    painter.drawText(2, 14, '[{}, {})'.format(slice_start, slice_stop))
                    '''

                    pc[i] = pixmap
                else:
                    refresh_required = True

            if i in pc:
                yield dest_rect, src_rect, pc[i]

        # Schedule another update if we didn't get all the required image data yet
        if refresh_required:
            QObject.emit(self, SIGNAL('refresh()'))

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(QColor(0, 0, 0))
        painter.eraseRect(0, 0, self.width(), self.height())

        if self._length == 0:
            return

        for dest_rect, src_rect, pixmap in self._get_visible_pixmaps():
            if pixmap:
                painter.drawPixmap(dest_rect, pixmap, src_rect)

        end = time.time()
        elapsed = end - start
        #print('Sample view updated in {:.2f} ms'.format(elapsed * 1000))

    def resizeEvent(self, event):
        self.update()

    def _refresh(self):
        self.update()


class DummyShape():
    '''Temporary type for testing purposes'''


class Shape():

    def __init__(self):
        self._shapes = []

    def draw_shape(self, painter):
        if not self._shapes:
            return

        ch_count = len(self._shapes)

        painter.save()
        painter.scale(1, 1 / ch_count)
        painter.translate(0, -ch_count + 1)

        for shape in self._shapes:
            if isinstance(shape, QPolygonF):
                painter.drawPolygon(shape)
            elif isinstance(shape, QPainterPath):
                painter.setBrush(Qt.NoBrush)
                painter.drawPath(shape)
            painter.translate(0, 2)

        painter.restore()

    def make_shape(self, sample_data, ref_fpp, slice_range):
        if not sample_data:
            return

        start, stop = slice_range
        slice_range_width = stop - start
        assert slice_range_width > 0
        actual_stop = min(stop, len(sample_data[0]))

        if ref_fpp > 1:
            # Filled blob
            ref_fpp_i = int(ref_fpp)
            for ch_data in sample_data:
                min_vals = []
                max_vals = []

                subslice_start = start
                while subslice_start < actual_stop:
                    subslice_stop = min(subslice_start + ref_fpp_i, actual_stop)
                    sl = ch_data[subslice_start:subslice_stop]
                    min_val = max(-1.0, min(sl))
                    max_val = min(1.0, max(sl))
                    min_vals.append(-min_val)
                    max_vals.append(-max_val)
                    subslice_start = subslice_stop

                min_vals = list(reversed(min_vals))
                sample_count = len(min_vals)

                points = [QPointF(i, val) for i, val in enumerate(max_vals)]
                points.extend([QPointF(sample_count - i - 1, val)
                    for i, val in enumerate(min_vals)])

                shape = QPolygonF(points)
                self._shapes.append(shape)

        else:
            # Line
            frame_step = 1 / ref_fpp
            for ch_data in sample_data:
                shape = QPainterPath()

                prev_val = ch_data[start - 1] if start > 0 else 0.0
                prev_point = QPointF(-frame_step * 0.5, -prev_val)
                shape.moveTo(prev_point)

                for i, val in enumerate(ch_data[start:stop]):
                    val = min(max(-1, -val), 1)
                    point = QPointF((i + 0.5) * frame_step, val)
                    shape.lineTo(point)

                next_val = ch_data[stop] if stop < len(ch_data) else 0.0
                next_point = QPointF((slice_range_width + 0.5) * frame_step, -next_val)
                shape.lineTo(next_point)

                self._shapes.append(shape)


class ShapeWorker():

    def __init__(self):
        self._shapes = {}
        self._requests = []
        self._cur_shape_prio = 0

        self._sample_length = 0
        self._get_sample_data = None
        self._sample_data = []

    def reset(self, length, get_sample_data):
        self._sample_length = length
        self._get_sample_data = get_sample_data
        self._sample_data = []

        self._shapes = {}
        self._requests = []
        self._cur_shape_prio = 0

    def _try_make_shape(self, ref_fpp, slice_range):
        start, stop = slice_range
        assert start < self._sample_length

        # Get new sample data if needed
        if not self._sample_data or (len(self._sample_data[0]) < stop + 1):
            new_data = self._get_sample_data()
            if not self._sample_data:
                self._sample_data = list(new_data)
            else:
                assert len(self._sample_data) == len(new_data)
                for i, new_ch_data in enumerate(new_data):
                    self._sample_data[i].extend(new_ch_data)

        read_count = len(self._sample_data[0]) if self._sample_data else 0
        if read_count < stop + 1 and read_count < self._sample_length:
            return None

        shape = Shape()
        shape.make_shape(self._sample_data, ref_fpp, slice_range)
        return shape

    def update(self):
        if not self._requests:
            self._cur_shape_prio = 0
            return

        # Give priority to future requests as they are
        # probably more relevant to current view
        self._cur_shape_prio -= 1

        time_limit = 0.01
        start_time = None

        while (start_time == None or
                time.time() - start_time < time_limit) and self._requests:
            if start_time == None:
                start_time = time.time()

            prio, request = heapq.heappop(self._requests)

            ref_fpp, slice_range = request
            shape = self._try_make_shape(ref_fpp, slice_range)
            if shape:
                key = (ref_fpp, slice_range)
                assert key not in self._shapes
                self._shapes[key] = shape
            else:
                # Shape is not finished yet, so push the request back
                heapq.heappush(self._requests, (prio + 1, request))

    def request_shape(self, ref_fpp, slice_range):
        # Return a completed shape
        key = (ref_fpp, slice_range)
        if key in self._shapes:
            return self._shapes[key]

        # Check if we are working on the requested shape
        if any(1 for p, k in self._requests if k == key):
            return None

        # Add request for a new shape
        heapq.heappush(self._requests, (self._cur_shape_prio, key))
        return None


