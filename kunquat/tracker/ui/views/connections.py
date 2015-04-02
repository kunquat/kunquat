# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2015
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import defaultdict
from itertools import izip
import math
import time

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from linesegment import LineSegment


_title_font = QFont(QFont().defaultFamily(), 10)
_title_font.setWeight(QFont.Bold)

_port_font = QFont(QFont().defaultFamily(), 8)
_port_font.setWeight(QFont.Bold)


DEFAULT_CONFIG = {
        'bg_colour'               : QColor(0x11, 0x11, 0x11),
        'edge_colour'             : QColor(0xcc, 0xcc, 0xcc),
        'focused_edge_colour'     : QColor(0xff, 0x88, 0x44),
        'focused_edge_width'      : 3,
        'edge_focus_dist_max'     : 4,
        'invalid_port_colour'     : QColor(0xff, 0x33, 0x33),
        'invalid_port_line_width' : 3,
        'invalid_port_marker_size': 13,
        'devices': {
            'width'              : 100,
            'title_font'         : _title_font,
            'port_font'          : _port_font,
            'port_handle_size'   : 7,
            'port_focus_dist_max': 5,
            'port_colour'        : QColor(0xee, 0xcc, 0xaa),
            'focused_port_colour': QColor(0xff, 0x77, 0x22),
            'padding'            : 4,
            'button_width'       : 50,
            'button_padding'     : 2,
            'audio_unit': {
                'bg_colour'       : QColor(0x33, 0x33, 0x55),
                'fg_colour'       : QColor(0xdd, 0xee, 0xff),
                'button_bg_colour': QColor(0x11, 0x11, 0x33),
                'button_focused_bg_colour': QColor(0, 0, 0x77),
            },
            'processor': {
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


# Vector utils needed for distance calculations

class Vec(tuple):

    def __new__(cls, coords):
        return tuple.__new__(cls, (float(x) for x in coords))

    def __add__(self, other):
        return Vec(x + y for (x, y) in izip(self, other))

    def __sub__(self, other):
        return Vec(x - y for (x, y) in izip(self, other))

    def __mul__(self, other):
        return Vec(x * other for x in self)

    def __rmul__(self, other):
        return self.__mul__(other)

def dot(a, b):
    return sum(x * y for (x, y) in izip(a, b))

def norm_sq(a):
    return sum(x * x for x in a)

def norm(a):
    return math.sqrt(norm_sq(a))

def dist(a, b):
    return norm(a - b)


def get_dist_to_ls(point, ls_a, ls_b):
    x = Vec(point)
    a = Vec(ls_a)
    b = Vec(ls_b)

    length_sq = norm_sq(a - b)
    if length_sq < 0.1:
        return dist(point, a)

    t = dot(x - a, b - a) / length_sq

    # Check cases where point does not intersect with a normal of the line segment
    if t < 0:
        return dist(x, a)
    elif t > 1:
        return dist(x, b)

    # Return distance to the projection on the line segment
    proj = a + t * (b - a)
    return dist(x, proj)


class Connections(QAbstractScrollArea):

    def __init__(self):
        QAbstractScrollArea.__init__(self)

        self.setViewport(ConnectionsView())
        self.viewport().setFocusProxy(None)

        self.horizontalScrollBar().setSingleStep(8)
        self.verticalScrollBar().setSingleStep(8)

    def set_au_id(self, au_id):
        self.viewport().set_au_id(au_id)

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


class EdgeMenu(QMenu):

    def __init__(self, parent):
        QMenu.__init__(self, parent)
        self.hide()
        self.addAction('Remove')
        self._edge = None

    def show_with_edge(self, edge, position):
        self._edge = edge
        self.popup(position)

    def get_edge(self):
        return self._edge


STATE_IDLE = 'idle'
STATE_MOVING = 'moving'
STATE_PRESSING = 'pressing'
STATE_EDGE_MENU = 'edge_menu'
STATE_ADDING_EDGE = 'adding_edge'


class ConnectionsView(QWidget):

    positionsChanged = pyqtSignal(name='positionsChanged')

    def __init__(self, config={}):
        QWidget.__init__(self)
        self._ui_model = None
        self._au_id = None
        self._updater = None

        self._state = STATE_IDLE

        self._visible_device_ids = []
        self._visible_devices = {}

        self._center_pos = (0, 0)

        self._focused_id = None
        self._focused_rel_pos = (0, 0)

        self._focused_button_info = {}
        self._pressed_button_info = {}

        self._focused_port_info = {}
        self._adding_edge_info = {}

        self._focused_edge_info = {}

        self._default_offsets = {}

        self._ls_cache = {}

        self._edge_menu = EdgeMenu(self)
        QObject.connect(
                self._edge_menu, SIGNAL('aboutToHide()'), self._edge_menu_closing)
        QObject.connect(
                self._edge_menu, SIGNAL('triggered(QAction*)'), self._remove_edge)

        self._config = None
        self._set_config(config)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setFocusPolicy(Qt.ClickFocus)
        self.setMouseTracking(True)

    def set_au_id(self, au_id):
        assert self._ui_model == None, "Cannot set audio unit ID after UI model"
        self._au_id = au_id

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

        if self._au_id != None:
            au = module.get_audio_unit(self._au_id)
            return au.get_connections()

        return module.get_connections()

    def _get_signal(self, base):
        parts = [base]
        if self._au_id != None:
            parts.append(self._au_id)
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
        update_signals = set([
            'signal_module',
            self._get_signal('signal_connections'),
            'signal_controls',
            ])
        if not signals.isdisjoint(update_signals):
            self._update_devices()

    def _get_device(self, dev_id):
        container = self._ui_model.get_module()
        if self._au_id != None:
            container = container.get_audio_unit(self._au_id)

        if dev_id.startswith('au'):
            return container.get_audio_unit(dev_id)
        elif dev_id.startswith('proc'):
            return container.get_processor(dev_id)

        return container

    def _get_in_ports(self, dev_id):
        device = self._get_device(dev_id)
        return device.get_in_ports()

    def _get_out_ports(self, dev_id):
        device = self._get_device(dev_id)
        return device.get_out_ports()

    def _get_device_name(self, dev_id):
        device = self._get_device(dev_id)
        return device.get_name()

    def _update_devices(self):
        connections = self._get_connections()
        layout = connections.get_layout()

        self._center_pos = layout.get('center_pos', (0, 0))

        # Get visible device IDs
        visible_set = set(['master'])

        module = self._ui_model.get_module()
        if self._au_id != None:
            visible_set |= set(['Iin'])

            au = module.get_audio_unit(self._au_id)
            proc_ids = au.get_processor_ids()
            existent_proc_ids = [proc_id for proc_id in proc_ids
                    if au.get_processor(proc_id).get_existence()]
            visible_set |= set(existent_proc_ids)

            eff_ids = au.get_au_ids()
            existent_eff_ids = [eff_id for eff_id in eff_ids
                    if au.get_au(eff_id).get_existence()]
            visible_set |= set(existent_eff_ids)

        else:
            au_ids = module.get_au_ids()
            existent_au_ids = [au_id for au_id in au_ids
                    if module.get_audio_unit(au_id).get_existence()]
            visible_set |= set(existent_au_ids)

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
                old_device = self._visible_devices[dev_id]
                if old_device.get_name() == self._get_device_name(dev_id):
                    new_devices[dev_id] = old_device
        self._visible_devices = new_devices

        QObject.emit(self, SIGNAL('positionsChanged()'))
        self.update()

    def _split_path(self, path):
        parts = path.split('/')
        port_id = parts[-1]
        if len(parts) > 1:
            dev_id = parts[0]
        else:
            dev_id = 'master' if port_id.startswith('out') else 'Iin'
        return (dev_id, port_id)

    def _get_port_center_from_path(self, path):
        dev_id, port_id = self._split_path(path)
        port_center = self._visible_devices[dev_id].get_port_center(port_id)
        return port_center

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

        # Make devices
        mid_offset = -200 if (self._au_id == None) else 0
        default_pos_cfg = {
                'au':       { 'index': 0, 'offset_x': mid_offset,   'offset_y': 120 },
                'proc':     { 'index': 0, 'offset_x': mid_offset,   'offset_y': 120 },
                'eff':      { 'index': 0, 'offset_x': 0,            'offset_y': 120 },
                'master':   { 'index': 0, 'offset_x': 200,          'offset_y': 120 },
                'Iin':      { 'index': 0, 'offset_x': -200,         'offset_y': 120 },
            }

        new_visible_devices = {}

        for dev_id in self._visible_device_ids:
            if dev_id in self._visible_devices:
                new_visible_devices[dev_id] = self._visible_devices[dev_id]
            else:
                if dev_id == 'master':
                    in_ports = self._get_out_ports(dev_id)
                    out_ports = []
                elif dev_id == 'Iin':
                    in_ports = []
                    out_ports = self._get_in_ports(dev_id)
                else:
                    in_ports = self._get_in_ports(dev_id)
                    out_ports = self._get_out_ports(dev_id)

                device = Device(
                        dev_id,
                        self._config['devices'],
                        in_ports,
                        out_ports,
                        self._get_device_name(dev_id))
                device.draw_pixmaps()
                new_visible_devices[dev_id] = device

            dev_layout = layout.get(dev_id, {})
            if 'offset' in dev_layout:
                offset = dev_layout['offset']
            else:
                # Get a default position
                index_key = dev_id.split('_')[0]
                pos_cfg = default_pos_cfg[index_key]
                y_offset_factor = (pos_cfg['index'] + 1) // 2
                y_offset_factor *= (-1 if (pos_cfg['index'] % 2 == 1) else 1)
                pos_cfg['index'] += 1
                offset = (
                        self._center_pos[0] + pos_cfg['offset_x'],
                        self._center_pos[1] + y_offset_factor * pos_cfg['offset_y'])
                self._default_offsets[dev_id] = offset

            device = new_visible_devices[dev_id]
            device.set_offset(offset)

        self._visible_devices = new_visible_devices

        # Draw connections
        new_ls_cache = {}

        edges = connections.get_connections()
        for edge in edges:
            from_path, to_path = edge
            from_pos = self._get_port_center_from_path(from_path)
            to_pos = self._get_port_center_from_path(to_path)
            key = (from_pos, to_pos)
            if key in self._ls_cache:
                new_ls_cache[key] = self._ls_cache[key]
            else:
                ls = LineSegment(from_pos, to_pos)
                ls.set_colour(self._config['edge_colour'])
                ls.draw_line()
                new_ls_cache[key] = ls

        self._ls_cache = new_ls_cache
        for ls in self._ls_cache.itervalues():
            ls.copy_line(painter)

        # Highlight focused connection
        if self._focused_edge_info:
            from_path, to_path = self._focused_edge_info['paths']
            from_x, from_y = self._get_port_center_from_path(from_path)
            to_x, to_y = self._get_port_center_from_path(to_path)
            edge_width = self._config['focused_edge_width']
            offset = edge_width // 2
            from_x, from_y = from_x + offset, from_y + offset
            to_x, to_y = to_x + offset, to_y + offset

            painter.save()
            painter.translate(-0.5, -0.5)
            pen = QPen(self._config['focused_edge_colour'])
            pen.setWidth(edge_width)
            painter.setPen(pen)
            painter.setRenderHint(QPainter.Antialiasing)
            painter.drawLine(from_x, from_y, to_x, to_y)
            painter.restore()

        # Draw connection that is being added
        if self._adding_edge_info:
            from_info = self._adding_edge_info['from']
            from_dev_id = from_info['dev_id']
            from_port = from_info['port']
            from_pos = self._visible_devices[from_dev_id].get_port_center(from_port)
            from_x, from_y = from_pos

            pen = QPen(self._config['focused_edge_colour'])
            to_info = self._adding_edge_info['to']

            if to_info:
                to_dev_id = to_info['dev_id']
                to_port = to_info['port']
                to_pos = self._visible_devices[to_dev_id].get_port_center(to_port)
                to_x, to_y = to_pos
                pen.setWidth(self._config['focused_edge_width'])
            else:
                to_pos = self._adding_edge_info['mouse_pos']
                to_x, to_y = to_pos

            painter.save()
            painter.translate(0.5, 0.5)
            painter.setPen(pen)
            painter.setRenderHint(QPainter.Antialiasing)
            painter.drawLine(from_x, from_y, to_x, to_y)
            painter.restore()

        # Draw devices
        for dev_id in self._visible_device_ids:
            device = self._visible_devices[dev_id]
            device.copy_pixmaps(painter)

            focus_info = self._focused_port_info
            if self._adding_edge_info:
                # Highlight port(s) of a new connection being added
                from_info = self._adding_edge_info['from']
                to_info = self._adding_edge_info['to']
                if from_info.get('dev_id') == dev_id:
                    focus_info = from_info
                elif to_info.get('dev_id') == dev_id:
                    focus_info = to_info

            device.draw_ports(painter, focus_info)

            # Draw button highlight
            if self._focused_button_info.get('dev_id') == dev_id:
                if ((not self._focused_button_info.get('pressed')) or
                        (self._focused_button_info == self._pressed_button_info)):
                    device.draw_button_highlight(painter, self._focused_button_info)

        # Mark attempt to make an invalid connection
        invalid_target_info = self._adding_edge_info.get('to_invalid')
        if invalid_target_info:
            dev_id = invalid_target_info['dev_id']
            port_id = invalid_target_info['port']
            pos = self._visible_devices[dev_id].get_port_center(port_id)
            pos_x, pos_y = pos

            pen = QPen(
                    self._config['invalid_port_colour'],
                    self._config['invalid_port_line_width'])
            ofs = self._config['invalid_port_marker_size'] // 2

            painter.save()
            painter.translate(0.5, 0.5)
            painter.setRenderHint(QPainter.Antialiasing)
            painter.setPen(pen)
            painter.drawLine(pos_x - ofs, pos_y - ofs, pos_x + ofs, pos_y + ofs)
            painter.drawLine(pos_x + ofs, pos_y - ofs, pos_x - ofs, pos_y + ofs)
            painter.restore()

        end = time.time()
        elapsed = end - start
        #print('Connections view updated in {:.2f} ms'.format(elapsed * 1000))

    def _get_area_pos(self, widget_x, widget_y):
        return (widget_x - self.width() // 2 + self._center_pos[0],
                widget_y - self.height() // 2 + self._center_pos[1])

    def _edge_menu_closing(self):
        self._state = STATE_IDLE
        self._focused_edge_info = {}

    def _remove_edge(self, action):
        edge = self._edge_menu.get_edge()
        connections = self._get_connections()
        edges = connections.get_connections()
        edges.remove(edge)
        connections.set_connections(edges)
        self._updater.signal_update(set([self._get_signal('signal_connections')]))
        self.update()

    def _is_send_port(self, dev_id, port_id):
        is_out = port_id.startswith('out')
        if dev_id in ('master', 'Iin'):
            return not is_out
        return is_out

    def _make_path(self, port_info):
        parts = []
        if port_info['dev_id'] not in ('master', 'Iin'):
            parts.append(port_info['dev_id'])
        if port_info['dev_id'].startswith('proc'):
            parts.append('C')
        parts.append(port_info['port'])
        return '/'.join(parts)

    def _edge_exists(self, from_info, to_info):
        if not self._is_send_port(from_info['dev_id'], from_info['port']):
            from_info, to_info = to_info, from_info
        from_path = self._make_path(from_info)
        to_path = self._make_path(to_info)
        edges = self._get_connections().get_connections()
        return [from_path, to_path] in edges

    def _edge_completes_cycle(self, from_info, to_info):
        if not self._is_send_port(from_info['dev_id'], from_info['port']):
            from_info, to_info = to_info, from_info

        # Build adjacency lists
        adj_lists = defaultdict(lambda: [], { from_info['dev_id']: [to_info['dev_id']] })
        edges = self._get_connections().get_connections()
        for edge in edges:
            from_path, to_path = edge
            from_dev_id, _ = self._split_path(from_path)
            to_dev_id, _ = self._split_path(to_path)
            adj_lists[from_dev_id].append(to_dev_id)

        def find_device(cur_dev_id, key_dev_id):
            if cur_dev_id == key_dev_id:
                return True
            return any(find_device(next_id, key_dev_id)
                    for next_id in adj_lists[cur_dev_id])

        return find_device(to_info['dev_id'], from_info['dev_id'])

    def _edge_violates_vf_cut(self, from_info, to_info):
        if not self._is_send_port(from_info['dev_id'], from_info['port']):
            from_info, to_info = to_info, from_info

        if to_info['dev_id'] == 'master':
            if from_info['dev_id'].startswith('proc_'):
                proc = self._get_device(from_info['dev_id'])
                port_num = int(from_info['port'].split('_')[1], 16)
                if not proc.get_vf_cut(port_num):
                    return True

        return False

    def mouseMoveEvent(self, event):
        area_pos = self._get_area_pos(event.x(), event.y())

        new_focused_port_info = {}
        new_focused_button_info = {}
        new_focused_edge_info = {}

        if self._state == STATE_IDLE:
            # Look for a focused part of a device
            on_device = False
            for dev_id in reversed(self._visible_device_ids):
                device = self._visible_devices[dev_id]
                dev_rel_pos = device.get_rel_pos(area_pos)
                focused_port = device.get_port_at(dev_rel_pos)
                if focused_port:
                    on_device = True
                    new_focused_port_info = { 'dev_id': dev_id, 'port': focused_port }
                    break
                elif device.contains_rel_pos(dev_rel_pos):
                    on_device = True
                    if device.has_button_at(dev_rel_pos):
                        new_focused_button_info = { 'dev_id': dev_id }
                    break

            if not on_device:
                # Find a focused edge
                connections = self._get_connections()
                edges = connections.get_connections()
                for edge in edges:
                    from_path, to_path = edge
                    from_pos = self._get_port_center_from_path(from_path)
                    to_pos = self._get_port_center_from_path(to_path)
                    dist = get_dist_to_ls(area_pos, from_pos, to_pos)
                    if dist <= self._config['edge_focus_dist_max']:
                        new_focused_edge_info = { 'paths': edge }

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

        elif self._state == STATE_ADDING_EDGE:
            self._adding_edge_info['mouse_pos'] = area_pos
            self._adding_edge_info['to'] = {}
            self._adding_edge_info['to_invalid'] = {}

            from_info = self._adding_edge_info['from']
            is_from_send = self._is_send_port(from_info['dev_id'], from_info['port'])

            # Look for a suitable target port
            for dev_id in reversed(self._visible_device_ids):
                device = self._visible_devices[dev_id]
                dev_rel_pos = device.get_rel_pos(area_pos)
                port_id = device.get_port_at(dev_rel_pos)
                if port_id:
                    to_info = { 'dev_id': dev_id, 'port': port_id }
                    is_to_send = self._is_send_port(to_info['dev_id'], to_info['port'])

                    # Add suggested connection only if valid
                    if ((is_from_send != is_to_send) and
                            not self._edge_exists(from_info, to_info) and
                            not self._edge_completes_cycle(from_info, to_info) and
                            not self._edge_violates_vf_cut(from_info, to_info)):
                        self._adding_edge_info['to'] = to_info
                    elif to_info != from_info:
                        self._adding_edge_info['to_invalid'] = to_info

                    break

            self.update()

        # Only one focused thing at a time
        assert sum(1 for x in
                (new_focused_port_info, new_focused_button_info, new_focused_edge_info)
                if x) <= 1

        # Update focus info
        if self._focused_port_info != new_focused_port_info:
            self._focused_port_info = new_focused_port_info
            self.update()
        if self._focused_button_info != new_focused_button_info:
            self._focused_button_info = new_focused_button_info
            self.update()
        if self._focused_edge_info != new_focused_edge_info:
            self._focused_edge_info = new_focused_edge_info
            self.update()

    def mousePressEvent(self, event):
        assert self._state != STATE_EDGE_MENU

        area_pos = self._get_area_pos(event.x(), event.y())

        # Find out what was pressed
        for dev_id in reversed(self._visible_device_ids):
            device = self._visible_devices[dev_id]
            dev_rel_pos = device.get_rel_pos(area_pos)
            focused_port = device.get_port_at(dev_rel_pos)
            if focused_port or device.contains_rel_pos(dev_rel_pos):
                self._focused_id = dev_id
                self._focused_rel_pos = dev_rel_pos
                break

        # Raise focused device to the top
        if self._focused_id:
            new_visible_ids = self._visible_device_ids
            new_visible_ids.remove(self._focused_id)
            new_visible_ids.append(self._focused_id)
            self._change_layout_entry('z_order', new_visible_ids)

        if not self._focused_id:
            if self._focused_edge_info:
                self._state = STATE_EDGE_MENU
                self._edge_menu.show_with_edge(
                        self._focused_edge_info['paths'],
                        self.mapToGlobal(QPoint(event.x(), event.y())))

        # Update state
        if self._focused_id:
            device = self._visible_devices[self._focused_id]
            dev_rel_pos = device.get_rel_pos(area_pos)
            if device.has_button_at(dev_rel_pos):
                self._state = STATE_PRESSING
                self._pressed_button_info = {
                        'dev_id': self._focused_id, 'pressed': True }
                self._focused_button_info = self._pressed_button_info.copy()
            elif self._focused_port_info:
                self._state = STATE_ADDING_EDGE
                self._adding_edge_info = {
                    'from': self._focused_port_info, 'mouse_pos': area_pos, 'to': {},
                }
            else:
                self._state = STATE_MOVING

    def mouseReleaseEvent(self, event):
        if self._state == STATE_EDGE_MENU:
            return

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
        elif self._state == STATE_ADDING_EDGE:
            from_info = self._adding_edge_info['from']
            to_info = self._adding_edge_info['to']

            if from_info and to_info:
                if not self._is_send_port(from_info['dev_id'], from_info['port']):
                    from_info, to_info = to_info, from_info
                from_path = self._make_path(from_info)
                to_path = self._make_path(to_info)
                edge = [from_path, to_path]
                connections = self._get_connections()
                edges = connections.get_connections()
                edges.append(edge)
                connections.set_connections(edges)

            self._adding_edge_info = {}
            self._updater.signal_update(set([self._get_signal('signal_connections')]))
            self.update()

        self._state = STATE_IDLE

    def _perform_button_click(self, button_info):
        visibility_manager = self._ui_model.get_visibility_manager()
        dev_id = button_info['dev_id']
        if dev_id.startswith('au'):
            visibility_manager.show_audio_unit(dev_id)
        elif dev_id.startswith('proc'):
            visibility_manager.show_processor(self._au_id, dev_id)

    def leaveEvent(self, event):
        if self._state == STATE_EDGE_MENU:
            return

        self._focused_id = None
        self._focused_rel_pos = (0, 0)
        self._focused_button_info = {}
        self._pressed_button_info = {}
        self._focused_port_info = {}
        self._focused_edge_info = {}


class Device():

    def __init__(self, dev_id, config, in_ports, out_ports, name):
        self._id = dev_id
        self._config = config

        self._offset_x = 0
        self._offset_y = 0

        self._name = name

        if dev_id in ('master', 'Iin'):
            self._type_config = self._config['master']
        elif dev_id.startswith('au'):
            self._type_config = self._config['audio_unit']
        elif dev_id.startswith('proc'):
            self._type_config = self._config['processor']
        else:
            raise ValueError('Unexpected type of device ID: {}'.format(dev_id))

        self._in_ports = in_ports
        self._out_ports = out_ports

        self._bg = None

    def get_name(self):
        return self._name

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
        if self._id == 'master':
            title = 'Master Out'
        elif self._id == 'Iin':
            title = 'Master In'
        else:
            title = self._name or '-'
        painter.drawText(
                QRectF(0, pad, self._bg.width(), title_height), title, text_option)

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

    def get_port_center(self, port_id):
        if port_id.startswith('in') != (self._id in ('master', 'Iin')):
            for i, point in enumerate(self._get_in_port_centers()):
                if self._in_ports[i] == port_id:
                    return point
        else:
            for i, point in enumerate(self._get_out_port_centers()):
                if self._out_ports[i] == port_id:
                    return point

        assert False, 'Device {} does not have port {}'.format(self._id, port_id)

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

    def draw_ports(self, painter, focus_info):
        painter.save()

        normal_colour = self._config['port_colour']
        focused_colour = self._config['focused_port_colour']
        focused_port = (focus_info.get('port')
                if focus_info.get('dev_id') == self._id
                else None)

        for i, rect in enumerate(self._get_in_port_rects()):
            port_id = self._in_ports[i]
            colour = focused_colour if port_id == focused_port else normal_colour
            painter.fillRect(rect, colour)

        for i, rect in enumerate(self._get_out_port_rects()):
            port_id = self._out_ports[i]
            colour = focused_colour if port_id == focused_port else normal_colour
            painter.fillRect(rect, colour)

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

    def get_port_at(self, rel_pos):
        port_dist_max = self._config['port_focus_dist_max']
        dev_x = rel_pos[0] - 1
        dev_y = rel_pos[1] - 1

        x_dist_max = self._bg.width() // 2 + port_dist_max + 1
        y_dist_max = self._bg.height() // 2
        if not ((abs(dev_x) <= x_dist_max) and (abs(dev_y) <= y_dist_max)):
            return None

        if dev_x < 0:
            for i, point in enumerate(self._get_in_port_centers()):
                x, y = point
                x -= self._offset_x
                y -= self._offset_y
                if max(abs(dev_x - x), abs(dev_y - y)) <= port_dist_max:
                    return self._in_ports[i]
        else:
            for i, point in enumerate(self._get_out_port_centers()):
                x, y = point
                x -= self._offset_x
                y -= self._offset_y
                if max(abs(dev_x - x), abs(dev_y - y)) <= port_dist_max:
                    return self._out_ports[i]

        return None

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
        return ((self._id not in ('master', 'Iin')) and
                (self._id.startswith('au') or self._id.startswith('proc')))

    def _get_height(self):
        title_height = self._get_title_height()
        port_height = self._get_port_height()
        ports_height = max(len(self._in_ports), len(self._out_ports)) * port_height

        if ports_height > 0:
            total_height = title_height + ports_height
        else:
            total_height = title_height + self._config['padding'] * 2

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


