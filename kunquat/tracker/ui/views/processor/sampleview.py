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
        x_offset = (start_i * dest_rect_width) - view_start

        # Update worker
        self._shape_worker.update()

        refresh_required = False

        for i in range(start_i, stop_i):
            dest_rect = QRect(x_offset, 0, int(dest_rect_width), height)
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
                    test_rect = src_rect.adjusted(0, 0, -1, -1)
                    painter.drawRect(test_rect)

                    painter.drawText(2, 14, '[{}, {})'.format(slice_start, slice_stop))

                    pc[i] = pixmap
                else:
                    refresh_required = True

            if i in pc:
                yield dest_rect, src_rect, pc[i]

            x_offset += int(dest_rect_width)

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


class ShapeWorker():

    def __init__(self):
        self._shapes = {}
        self._requests = []
        self._cur_shape_prio = 0

        self._sample_length = 0
        self._get_sample_data = None
        self._sample_data = tuple()

    def reset(self, length, get_sample_data):
        self._sample_length = length
        self._get_sample_data = get_sample_data
        self._sample_data = tuple()

        self._shapes = {}
        self._requests = []
        self._cur_shape_prio = 0

    def _try_make_shape(self, ref_fpp, slice_range):
        time.sleep(0.03)
        return DummyShape()

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


