# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *


_title_font = QFont(QFont().defaultFamily(), 10)
_title_font.setWeight(QFont.Bold)

_port_font = QFont(QFont().defaultFamily(), 8)
_port_font.setWeight(QFont.Bold)


DEFAULT_CONFIG = {
        'bg_colour': QColor(0x11, 0x11, 0x11),
        'devices': {
            'title_font': _title_font,
            'port_font' : _port_font,
        }
    }


class Connections(QAbstractScrollArea):

    def __init__(self):
        QAbstractScrollArea.__init__(self)

        self.setViewport(ConnectionsView())
        self.viewport().setFocusProxy(None)

    def set_ui_model(self, ui_model):
        self.viewport().set_ui_model(ui_model)
        QObject.connect(
                self.viewport(), SIGNAL('positionsChanged()'), self._update_scrollbars)

    def unregister_updaters(self):
        self.viewport().unregister_updaters()

    def _update_scrollbars(self):
        visible_rect = self.viewport().get_visible_rect()
        area_rect = self.viewport().get_area_rect() or visible_rect
        full_rect = area_rect.united(visible_rect)

        hscrollbar = self.horizontalScrollBar()
        old_block = hscrollbar.blockSignals(True)
        hscrollbar.setPageStep(visible_rect.width())
        hscrollbar.setRange(
                full_rect.left(), full_rect.right() - self.viewport().width())
        hscrollbar.setValue(max(visible_rect.left(), full_rect.left()))
        hscrollbar.blockSignals(old_block)

        vscrollbar = self.verticalScrollBar()
        old_block = vscrollbar.blockSignals(True)
        vscrollbar.setPageStep(visible_rect.height())
        vscrollbar.setRange(
                full_rect.top(), full_rect.bottom() - self.viewport().height())
        vscrollbar.setValue(max(visible_rect.top(), full_rect.top()))
        vscrollbar.blockSignals(old_block)

        # Hack scroll bar visibility
        on = Qt.ScrollBarAlwaysOn
        off = Qt.ScrollBarAlwaysOff
        self.setHorizontalScrollBarPolicy(
                off if hscrollbar.minimum() == hscrollbar.maximum() else on)
        self.setVerticalScrollBarPolicy(
                off if vscrollbar.minimum() == vscrollbar.maximum() else on)

    def paintEvent(self, event):
        self.viewport().paintEvent(event)

    def mouseMoveEvent(self, event):
        self.viewport().mouseMoveEvent(event)

    def mousePressEvent(self, event):
        self.viewport().mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        self.viewport().mouseReleaseEvent(event)

    def resizeEvent(self, event):
        self._update_scrollbars()

    def scrollContentsBy(self, dx, dy):
        hscrollbar = self.horizontalScrollBar()
        biased_x = hscrollbar.value() - hscrollbar.minimum()
        vscrollbar = self.verticalScrollBar()
        biased_y = vscrollbar.value() - vscrollbar.minimum()
        self.viewport().scroll_to(biased_x, biased_y)


class ConnectionsView(QWidget):

    positionsChanged = pyqtSignal(name='positionsChanged')

    def __init__(self, config={}):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._visible_device_ids = []
        self._visible_devices = {}

        self._center_pos = (0, 0)

        self._focused_id = None
        self._focused_rel_pos = (0, 0)

        self._config = None
        self._set_config(config)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setFocusPolicy(Qt.ClickFocus)
        self.setMouseTracking(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_devices()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def get_area_rect(self):
        area_rect = None
        for device in self._visible_devices.itervalues():
            dev_rect = device.get_rect_in_area()
            if not area_rect:
                area_rect = dev_rect
            else:
                area_rect = area_rect.united(dev_rect)
        return area_rect

    def get_visible_rect(self):
        x_start = self._center_pos[0] - self.width() // 2
        y_start = self._center_pos[1] - self.height() // 2
        return QRect(x_start, y_start, self.width(), self.height())

    def scroll_to(self, area_x, area_y):
        area_rect = self.get_area_rect()
        visible_rect = self.get_visible_rect()
        if not area_rect or visible_rect.contains(area_rect, True):
            return

        full_rect = area_rect.united(visible_rect)
        self._center_pos = (
                full_rect.left() + self.width() // 2 + area_x,
                full_rect.top() + self.height() // 2 + area_y)

        QObject.emit(self, SIGNAL('positionsChanged()'))
        self.update()

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

    def _perform_updates(self, signals):
        if 'signal_module' in signals:
            self._update_devices()

    def _update_devices(self):
        module = self._ui_model.get_module()

        # Get visible device IDs
        visible_set = set(['master'])

        ins_ids = module.get_instrument_ids()
        existent_ins_ids = [ins_id for ins_id in ins_ids
                if module.get_instrument(ins_id).get_existence()]
        visible_set |= set(existent_ins_ids)

        eff_ids = module.get_effect_ids()
        existent_eff_ids = [eff_id for eff_id in eff_ids
                if module.get_effect(eff_id).get_existence()]
        visible_set |= set(existent_eff_ids)

        new_dev_ids = []

        # Add existent device IDs included in the z order
        z_order = [] # TODO: get from model
        for dev_id in z_order:
            if dev_id in visible_set:
                visible_set.remove(dev_id)
                new_dev_ids.append(dev_id)

        # Append remaining IDs
        remaining_ids = list(visible_set)
        for dev_id in remaining_ids:
            new_dev_ids.append(dev_id)

        # Update device info
        self._visible_device_ids = new_dev_ids

        new_devices = {}
        for dev_id in self._visible_device_ids:
            if dev_id in self._visible_devices:
                new_devices = self._visible_devices[dev_id]
        self._visible_devices = new_devices

        QObject.emit(self, SIGNAL('positionsChanged()'))
        self.update()

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        painter.translate(
                self.width() // 2 - self._center_pos[0],
                self.height() // 2 - self._center_pos[1])

        # Draw devices
        for dev_id in self._visible_device_ids:
            if dev_id not in self._visible_devices:
                device = Device(dev_id, self._config['devices'])
                device.draw_pixmaps()
                self._visible_devices[dev_id] = device
            self._visible_devices[dev_id].copy_pixmaps(painter)

        end = time.time()
        elapsed = end - start
        print('Connections view updated in {:.2f} ms'.format(elapsed * 1000))

    def _get_area_pos(self, widget_x, widget_y):
        return (widget_x - self.width() // 2 + self._center_pos[0],
                widget_y - self.height() // 2 + self._center_pos[1])

    def mouseMoveEvent(self, event):
        if not self._focused_id or self._focused_id not in self._visible_devices:
            return

        # Move focused device
        area_x, area_y = self._get_area_pos(event.x(), event.y())
        new_offset_x = area_x - self._focused_rel_pos[0]
        new_offset_y = area_y - self._focused_rel_pos[1]

        self._visible_devices[self._focused_id].set_offset((new_offset_x, new_offset_y))

        QObject.emit(self, SIGNAL('positionsChanged()'))
        self.update()

    def mousePressEvent(self, event):
        area_pos = self._get_area_pos(event.x(), event.y())

        # Find out what was pressed
        for dev_id in reversed(self._visible_device_ids):
            device = self._visible_devices[dev_id]
            dev_rel_pos = device.get_rel_pos(area_pos)
            if self._visible_devices[dev_id].contains_rel_pos(dev_rel_pos):
                self._focused_id = dev_id
                self._focused_rel_pos = dev_rel_pos
                break

        # Raise focused device to the top
        if self._focused_id:
            self._visible_device_ids.remove(self._focused_id)
            self._visible_device_ids.append(self._focused_id)
            self.update()

    def mouseReleaseEvent(self, event):
        self._focused_id = None
        self._focused_rel_pos = (0, 0)


class Device():

    def __init__(self, dev_id, config):
        self._id = dev_id
        self._config = config

        self._offset_x = 0
        self._offset_y = 0

        self._bg = None

    def draw_pixmaps(self):
        self._bg = QPixmap(100, 100)
        painter = QPainter(self._bg)

        # Test
        painter.setBackground(QColor(0, 0, 0))
        painter.eraseRect(0, 0, self._bg.width(), self._bg.height())
        painter.setPen(QColor(0xff, 0xff, 0xff))
        painter.drawRect(0, 0, self._bg.width() - 1, self._bg.height() - 1)

        # Title
        painter.setFont(self._config['title_font'])
        fm = QFontMetrics(self._config['title_font'])
        height = fm.boundingRect('Ag').height()
        text_option = QTextOption(Qt.AlignCenter)
        painter.drawText(QRectF(0, 0, self._bg.width(), height), self._id, text_option)

    def copy_pixmaps(self, painter):
        painter.save()

        painter.translate(self._offset_x, self._offset_y)

        bg_offset_x = -self._bg.width() // 2
        bg_offset_y = -self._bg.height() // 2
        painter.drawPixmap(bg_offset_x, bg_offset_y, self._bg)

        painter.restore()

    def set_offset(self, offset):
        self._offset_x, self._offset_y = offset

    def get_rel_pos(self, area_pos):
        return (area_pos[0] - self._offset_x, area_pos[1] - self._offset_y)

    def contains_rel_pos(self, rel_pos):
        x_dist_max = self._bg.width() // 2
        y_dist_max = self._bg.height() // 2
        return (abs(rel_pos[0]) <= x_dist_max) and (abs(rel_pos[1]) <= y_dist_max)

    def get_rect_in_area(self):
        return QRect(
                self._offset_x - self._bg.width() // 2,
                self._offset_y - self._bg.height() // 2,
                self._bg.width() + 1,
                self._bg.height() + 1)


