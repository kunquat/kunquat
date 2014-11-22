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
            'width'           : 100,
            'title_font'      : _title_font,
            'port_font'       : _port_font,
            'port_handle_size': 7,
            'port_colour'     : QColor(0xee, 0xcc, 0xaa),
            'padding'         : 4,
            'button_width'    : 50,
            'button_padding'  : 2,
            'instrument': {
                'bg_colour'       : QColor(0x33, 0x33, 0x55),
                'fg_colour'       : QColor(0xdd, 0xee, 0xff),
                'button_bg_colour': QColor(0x11, 0x11, 0x33),
                'button_focused_bg_colour': QColor(0, 0, 0x77),
            },
            'generator': {
                'bg_colour'       : QColor(0x22, 0x55, 0x55),
                'fg_colour'       : QColor(0xcc, 0xff, 0xff),
                'button_bg_colour': QColor(0x11, 0x33, 0x33),
                'button_focused_bg_colour': QColor(0, 0x55, 0x55),
            },
            'effect': {
                'bg_colour'       : QColor(0x55, 0x44, 0x33),
                'fg_colour'       : QColor(0xff, 0xee, 0xdd),
                'button_bg_colour': QColor(0x33, 0x22, 0x11),
                'button_focused_bg_colour': QColor(0x77, 0x22, 0),
            },
            'master': {
                'bg_colour'       : QColor(0x33, 0x55, 0x33),
                'fg_colour'       : QColor(0xdd, 0xff, 0xdd),
                'button_bg_colour': QColor(0x11, 0x33, 0x11),
                'button_focused_bg_colour': QColor(0, 0x77, 0),
            },
        },
    }


class Connections(QAbstractScrollArea):

    def __init__(self):
        QAbstractScrollArea.__init__(self)

        self.setViewport(ConnectionsView())
        self.viewport().setFocusProxy(None)

        self.horizontalScrollBar().setSingleStep(8)
        self.verticalScrollBar().setSingleStep(8)

    def set_ins_id(self, ins_id):
        self.viewport().set_ins_id(ins_id)

    def set_ui_model(self, ui_model):
        self.viewport().set_ui_model(ui_model)
        QObject.connect(
                self.viewport(), SIGNAL('positionsChanged()'), self._update_scrollbars)
        self._update_scrollbars()

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

    def mouseDoubleClickEvent(self, event):
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


STATE_IDLE = 'idle'
STATE_MOVING = 'moving'
STATE_PRESSING = 'pressing'


class ConnectionsView(QWidget):

    positionsChanged = pyqtSignal(name='positionsChanged')

    def __init__(self, config={}):
        QWidget.__init__(self)
        self._ui_model = None
        self._ins_id = None
        self._updater = None

        self._state = STATE_IDLE

        self._visible_device_ids = []
        self._visible_devices = {}

        self._center_pos = (0, 0)

        self._focused_id = None
        self._focused_rel_pos = (0, 0)

        self._focused_button_info = {}
        self._pressed_button_info = {}

        self._default_offsets = {}

        self._config = None
        self._set_config(config)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setFocusPolicy(Qt.ClickFocus)
        self.setMouseTracking(True)

    def set_ins_id(self, ins_id):
        assert self._ui_model == None, "Cannot set instrument ID after UI model"
        self._ins_id = ins_id

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

    def _get_connections(self):
        module = self._ui_model.get_module()

        if self._ins_id != None:
            instrument = module.get_instrument(self._ins_id)
            return instrument.get_connections()

        return module.get_connections()

    def _get_signal(self, base):
        parts = [base]
        if self._ins_id != None:
            parts.append(self._ins_id)
        return '_'.join(parts)

    def _change_layout_entry(self, key, value):
        connections = self._get_connections()
        layout = connections.get_layout()

        # Set new entry
        layout[key] = value

        # Also set entries for default offsets that were not changed
        for dev_id, offset in self._default_offsets.iteritems():
            if (dev_id not in layout) or ('offset' not in layout[dev_id]):
                dev_layout = layout.get(dev_id, {})
                dev_layout['offset'] = offset
                layout[dev_id] = dev_layout
        self._default_offsets = {}

        connections.set_layout(layout)
        self._updater.signal_update(set([self._get_signal('signal_connections')]))

    def scroll_to(self, area_x, area_y):
        area_rect = self.get_area_rect()
        visible_rect = self.get_visible_rect()
        if not area_rect or visible_rect.contains(area_rect, True):
            return

        full_rect = area_rect.united(visible_rect)
        new_center_pos = (
                full_rect.left() + self.width() // 2 + area_x,
                full_rect.top() + self.height() // 2 + area_y)

        self._change_layout_entry('center_pos', new_center_pos)

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()
        self._config.update(config)

    def _perform_updates(self, signals):
        update_signals = set(['signal_module', self._get_signal('signal_connections')])
        if not signals.isdisjoint(update_signals):
            self._update_devices()

    def _get_device(self, dev_id):
        container = self._ui_model.get_module()
        if self._ins_id != None:
            container = container.get_instrument(self._ins_id)

        if dev_id.startswith('ins'):
            return container.get_instrument(dev_id)
        elif dev_id.startswith('gen'):
            return container.get_generator(dev_id)
        elif dev_id.startswith('eff'):
            return container.get_effect(dev_id)
        elif dev_id.startswith('dsp'):
            return container.get_dsp(dev_id)

        return container

    def _get_in_ports(self, dev_id):
        if dev_id.startswith('ins') or dev_id.startswith('gen'):
            return []

        device = self._get_device(dev_id)
        return device.get_in_ports()

    def _get_out_ports(self, dev_id):
        device = self._get_device(dev_id)
        return device.get_out_ports()

    def _update_devices(self):
        connections = self._get_connections()
        layout = connections.get_layout()

        self._center_pos = layout.get('center_pos', (0, 0))

        # Get visible device IDs
        visible_set = set(['master'])

        module = self._ui_model.get_module()
        if self._ins_id != None:
            instrument = module.get_instrument(self._ins_id)
            gen_ids = instrument.get_generator_ids()
            existent_gen_ids = [gen_id for gen_id in gen_ids
                    if instrument.get_generator(gen_id).get_existence()]
            visible_set |= set(existent_gen_ids)

            eff_ids = instrument.get_effect_ids()
            existent_eff_ids = [eff_id for eff_id in eff_ids
                    if instrument.get_effect(eff_id).get_existence()]
            visible_set |= set(existent_eff_ids)

        else:
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
        z_order = layout.get('z_order', [])
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
                new_devices[dev_id] = self._visible_devices[dev_id]
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

        connections = self._get_connections()
        layout = connections.get_layout()

        # Draw devices
        default_pos_cfg = {
                'i': { 'index': 0, 'offset_x': -200, 'offset_y': 120 },
                'g': { 'index': 0, 'offset_x': -200, 'offset_y': 120 },
                'e': { 'index': 0, 'offset_x': 0,    'offset_y': 120 },
                'm': { 'index': 0, 'offset_x': 200,  'offset_y': 120 },
            }
        for dev_id in self._visible_device_ids:
            if dev_id not in self._visible_devices:
                if dev_id == 'master':
                    in_ports = self._get_out_ports(dev_id)
                    out_ports = []
                else:
                    in_ports = self._get_in_ports(dev_id)
                    out_ports = self._get_out_ports(dev_id)

                device = Device(dev_id, self._config['devices'], in_ports, out_ports)
                device.draw_pixmaps()
                self._visible_devices[dev_id] = device

            dev_layout = layout.get(dev_id, {})
            if 'offset' in dev_layout:
                offset = dev_layout['offset']
            else:
                # Get a default position
                index_key = dev_id[0]
                pos_cfg = default_pos_cfg[index_key]
                y_offset_factor = (pos_cfg['index'] + 1) // 2
                y_offset_factor *= (-1 if (pos_cfg['index'] % 2 == 1) else 1)
                pos_cfg['index'] += 1
                offset = (
                        self._center_pos[0] + pos_cfg['offset_x'],
                        self._center_pos[1] + y_offset_factor * pos_cfg['offset_y'])
                self._default_offsets[dev_id] = offset

            device = self._visible_devices[dev_id]
            device.set_offset(offset)
            device.copy_pixmaps(painter)

            device.draw_ports(painter)

            # Draw button highlight
            if self._focused_button_info.get('dev_id') == dev_id:
                if ((not self._focused_button_info.get('pressed')) or
                        (self._focused_button_info == self._pressed_button_info)):
                    device.draw_button_highlight(painter, self._focused_button_info)

        end = time.time()
        elapsed = end - start
        print('Connections view updated in {:.2f} ms'.format(elapsed * 1000))

    def _get_area_pos(self, widget_x, widget_y):
        return (widget_x - self.width() // 2 + self._center_pos[0],
                widget_y - self.height() // 2 + self._center_pos[1])

    def mouseMoveEvent(self, event):
        area_pos = self._get_area_pos(event.x(), event.y())

        new_focused_button_info = {}

        if self._state == STATE_IDLE:
            for dev_id in reversed(self._visible_device_ids):
                device = self._visible_devices[dev_id]
                dev_rel_pos = device.get_rel_pos(area_pos)
                if device.contains_rel_pos(dev_rel_pos):
                    if device.has_button_at(dev_rel_pos):
                        new_focused_button_info = { 'dev_id': dev_id }
                    break

        if self._state == STATE_MOVING:
            if (not self._focused_id) or (self._focused_id not in self._visible_devices):
                self._state = STATE_IDLE
            else:
                # Move focused device
                area_x, area_y = area_pos
                new_offset_x = area_x - self._focused_rel_pos[0]
                new_offset_y = area_y - self._focused_rel_pos[1]

                focused_layout = { 'offset': (new_offset_x, new_offset_y) }
                self._change_layout_entry(self._focused_id, focused_layout)

        elif self._state == STATE_PRESSING:
            if (not self._focused_id) or (self._focused_id not in self._visible_devices):
                self._state = STATE_IDLE
            else:
                # Keep track of focused button visuals
                dev_id = self._focused_id
                device = self._visible_devices[dev_id]
                dev_rel_pos = device.get_rel_pos(area_pos)
                if device.contains_rel_pos(dev_rel_pos):
                    if (device.has_button_at(dev_rel_pos) and
                            self._pressed_button_info.get('dev_id') == dev_id):
                        new_focused_button_info = {
                            'dev_id': dev_id,
                            'pressed': True,
                        }

        # Update button focus info
        if self._focused_button_info != new_focused_button_info:
            self._focused_button_info = new_focused_button_info
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
            new_visible_ids = self._visible_device_ids
            new_visible_ids.remove(self._focused_id)
            new_visible_ids.append(self._focused_id)
            self._change_layout_entry('z_order', new_visible_ids)

        # Update state
        if self._focused_id:
            device = self._visible_devices[self._focused_id]
            dev_rel_pos = device.get_rel_pos(area_pos)
            if device.has_button_at(dev_rel_pos):
                self._state = STATE_PRESSING
                self._pressed_button_info = {
                        'dev_id': self._focused_id, 'pressed': True }
                self._focused_button_info = self._pressed_button_info.copy()
            else:
                self._state = STATE_MOVING

    def mouseReleaseEvent(self, event):
        self._focused_id = None
        self._focused_rel_pos = (0, 0)

        if self._state == STATE_MOVING:
            pass
        elif self._state == STATE_PRESSING:
            clicked = (self._focused_button_info == self._pressed_button_info)
            if clicked:
                self._perform_button_click(self._focused_button_info)

            self._focused_button_info['pressed'] = False
            self._pressed_button_info = {}

            self.update()

        self._state = STATE_IDLE

    def _perform_button_click(self, button_info):
        visibility_manager = self._ui_model.get_visibility_manager()
        dev_id = button_info['dev_id']
        if dev_id.startswith('ins'):
            visibility_manager.show_instrument(dev_id)

    def leaveEvent(self, event):
        self._focused_id = None
        self._focused_rel_pos = (0, 0)
        self._focused_button_info = {}
        self._pressed_button_info = {}


class Device():

    def __init__(self, dev_id, config, in_ports, out_ports):
        self._id = dev_id
        self._config = config

        self._offset_x = 0
        self._offset_y = 0

        if dev_id == 'master':
            self._type_config = self._config['master']
        elif dev_id.startswith('ins'):
            self._type_config = self._config['instrument']
        elif dev_id.startswith('eff'):
            self._type_config = self._config['effect']
        elif dev_id.startswith('gen'):
            self._type_config = self._config['generator']
        else:
            raise ValueError('Unexpected type of device ID: {}'.format(dev_id))

        self._in_ports = in_ports
        self._out_ports = out_ports

        self._bg = None

    def draw_pixmaps(self):
        self._bg = QPixmap(self._config['width'], self._get_height())
        painter = QPainter(self._bg)
        pad = self._config['padding']

        # Background
        painter.setBackground(self._type_config['bg_colour'])
        painter.eraseRect(0, 0, self._bg.width(), self._bg.height())
        painter.setPen(self._type_config['fg_colour'])
        painter.drawRect(0, 0, self._bg.width() - 1, self._bg.height() - 1)

        # Title
        painter.setFont(self._config['title_font'])
        text_option = QTextOption(Qt.AlignCenter)
        title_height = self._get_title_height()
        painter.drawText(
                QRectF(0, pad, self._bg.width(), title_height), self._id, text_option)

        # Ports
        painter.setFont(self._config['port_font'])
        text_option = QTextOption(Qt.AlignLeft | Qt.AlignVCenter)
        port_height = self._get_port_height()
        port_y = self._get_title_height()

        for port_id in self._in_ports:
            port_num = int(port_id.split('_')[1], 16)
            painter.drawText(
                    QRectF(pad, port_y, self._bg.width() // 2, port_height),
                    str(port_num),
                    text_option)
            port_y += port_height

        text_option = QTextOption(Qt.AlignRight | Qt.AlignVCenter)
        port_y = self._get_title_height()
        for port_id in self._out_ports:
            port_num = int(port_id.split('_')[1], 16)
            painter.drawText(
                    QRectF(
                        self._bg.width() // 2,
                        port_y,
                        self._bg.width() // 2 - pad,
                        port_height),
                    str(port_num),
                    text_option)
            port_y += port_height

        # Edit button
        if self._has_edit_button():
            self._draw_edit_button(
                    painter,
                    self._type_config['button_bg_colour'],
                    self._type_config['fg_colour'])

    def copy_pixmaps(self, painter):
        painter.save()

        painter.translate(self._offset_x, self._offset_y)

        bg_offset_x, bg_offset_y = self._get_top_left_pos((0, 0))
        painter.drawPixmap(bg_offset_x, bg_offset_y, self._bg)

        painter.restore()

    def _get_in_port_centers(self):
        bg_offset_x, bg_offset_y = self._get_top_left_pos(
                (self._offset_x, self._offset_y))

        port_offset = self._config['port_handle_size'] // 2
        padding = self._config['padding']
        port_x = bg_offset_x - port_offset
        port_y = bg_offset_y + padding + self._get_title_height() + port_offset

        for _ in self._in_ports:
            yield (port_x, port_y)
            port_y += self._get_port_height()

    def _get_out_port_centers(self):
        bg_offset_x, bg_offset_y = self._get_top_left_pos(
                (self._offset_x, self._offset_y))

        port_offset = self._config['port_handle_size'] // 2
        padding = self._config['padding']
        port_x = bg_offset_x + self._bg.width() + port_offset - 1
        port_y = bg_offset_y + padding + self._get_title_height() + port_offset

        for _ in self._out_ports:
            yield (port_x, port_y)
            port_y += self._get_port_height()

    def _get_in_port_rects(self):
        handle_size = self._config['port_handle_size']
        handle_offset = -handle_size // 2 + 1
        for point in self._get_in_port_centers():
            x, y = point
            yield QRect(x + handle_offset, y + handle_offset, handle_size, handle_size)

    def _get_out_port_rects(self):
        handle_size = self._config['port_handle_size']
        handle_offset = -handle_size // 2 + 1
        for point in self._get_out_port_centers():
            x, y = point
            yield QRect(x + handle_offset, y + handle_offset, handle_size, handle_size)

    def draw_ports(self, painter):
        painter.save()

        for rect in self._get_in_port_rects():
            painter.fillRect(rect, self._config['port_colour'])

        for rect in self._get_out_port_rects():
            painter.fillRect(rect, self._config['port_colour'])

        painter.restore()

    def draw_button_highlight(self, painter, info):
        assert self._has_edit_button()
        painter.save()
        shift_x, shift_y = self._get_top_left_pos((0, 0))
        painter.translate(self._offset_x + shift_x, self._offset_y + shift_y)

        bg_colour = self._type_config['button_focused_bg_colour']
        fg_colour = self._type_config['fg_colour']
        if info.get('pressed'):
            bg_colour, fg_colour = fg_colour, bg_colour

        self._draw_edit_button(painter, bg_colour, fg_colour)

        painter.restore()

    def _get_top_left_pos(self, center_pos):
        center_x, center_y = center_pos
        return (center_x - self._bg.width() // 2, center_y - self._bg.height() // 2)

    def _get_dev_biased_pos(self, center_pos):
        center_x, center_y = center_pos
        return (center_x + self._bg.width() // 2, center_y + self._bg.height() // 2)

    def _draw_edit_button(self, painter, bg_colour, fg_colour):
        rect = self._get_edit_button_rect()

        painter.setPen(fg_colour)
        painter.setBrush(bg_colour)
        painter.drawRect(rect)

        painter.setFont(self._config['title_font'])
        text_option = QTextOption(Qt.AlignCenter)
        painter.drawText(QRectF(rect), 'Edit', text_option)

    def set_offset(self, offset):
        self._offset_x, self._offset_y = offset

    def get_rel_pos(self, area_pos):
        return (area_pos[0] - self._offset_x, area_pos[1] - self._offset_y)

    def contains_rel_pos(self, rel_pos):
        x_dist_max = self._bg.width() // 2
        y_dist_max = self._bg.height() // 2
        return (abs(rel_pos[0]) <= x_dist_max) and (abs(rel_pos[1]) <= y_dist_max)

    def has_button_at(self, rel_pos):
        if not self._has_edit_button():
            return False

        biased_x, biased_y = self._get_dev_biased_pos(rel_pos)
        rect = self._get_edit_button_rect()
        return (rect.left() <= biased_x <= rect.right() and
                rect.top() <= biased_y <= rect.bottom())

    def get_rect_in_area(self):
        return QRect(
                self._offset_x - self._bg.width() // 2,
                self._offset_y - self._bg.height() // 2,
                self._bg.width() + 1,
                self._bg.height() + 1)

    def _has_edit_button(self):
        # TODO: enable for generators & effects
        return (self._id != 'master') and (self._id.startswith('ins'))

    def _get_height(self):
        title_height = self._get_title_height()
        port_height = self._get_port_height()
        ports_height = max(len(self._in_ports), len(self._out_ports)) * port_height

        total_height = title_height + ports_height

        if self._has_edit_button():
            edit_button_height = self._get_edit_button_height()
            total_height += edit_button_height + self._config['padding']

        return total_height

    def _get_title_height(self):
        fm = QFontMetrics(self._config['title_font'])
        return fm.boundingRect('Ag').height()

    def _get_port_height(self):
        fm = QFontMetrics(self._config['port_font'])
        return self._config['padding'] + fm.boundingRect('0').height()

    def _get_edit_button_height(self):
        return self._get_title_height() + self._config['button_padding'] * 2

    def _get_edit_button_rect(self):
        height = self._get_edit_button_height()
        left = (self._config['width'] - self._config['button_width']) // 2
        top = self._get_height() - height - self._config['padding']
        return QRect(
                left, top,
                self._config['button_width'] - 1, height - 1)


