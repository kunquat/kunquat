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

    def unregister_updaters(self):
        self.viewport().unregister_updaters()

    def paintEvent(self, event):
        self.viewport().paintEvent(event)


class ConnectionsView(QWidget):

    def __init__(self, config={}):
        QWidget.__init__(self)
        self._ui_model = None
        self._updater = None

        self._visible_device_ids = []
        self._visible_devices = {}

        self._center_pos = (0, 0)

        self._config = None
        self._set_config(config)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setFocusPolicy(Qt.ClickFocus)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._update_devices()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

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

        # TODO: effect IDs

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

        self.update()

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        painter.translate(
                self.width() // 2 + self._center_pos[0],
                self.height() // 2 + self._center_pos[1])

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


