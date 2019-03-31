# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2014-2018
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

from collections import defaultdict
import math
import string
import time

from kunquat.tracker.ui.qt import *

from kunquat.tracker.ui.model.module import Module
from kunquat.tracker.ui.model.processor import Processor
from .audiounit.audiounitupdater import AudioUnitUpdater
from .confirmdialog import ConfirmDialog
from .connectioncable import ConnectionCable
from . import utils


_title_font = QFont(QFont().defaultFamily(), 10, QFont.Bold)
utils.set_glyph_rel_width(_title_font, QWidget, string.ascii_lowercase, 15.92)

_port_font = QFont(QFont().defaultFamily(), 8, QFont.Bold)
utils.set_glyph_rel_width(_port_font, QWidget, string.ascii_lowercase, 15.92)
_port_font.setPointSizeF(7.5)


DEFAULT_CONFIG = {
        'bg_colour'               : QColor(0x11, 0x11, 0x11),
        'edge_colour'             : QColor(0xcc, 0xcc, 0xcc),
        'edge_width'              : 1,
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
            'border_width'       : 1,
            'border_radius'      : 2,
            'padding'            : 4,
            'button_width'       : 44,
            'button_padding'     : 2,
            'hilight_border'     : 2,
            'hilight_padding'    : 2,
            'instrument': {
                'bg_colours'       : [QColor('#335'), QColor('#668'), QColor('#002')],
                'fg_colour'        : QColor(0xdd, 0xee, 0xff),
                'button_bg_colours': [QColor('#335'), QColor('#668'), QColor('#002')],
                'button_down_bg_colours':
                                     [QColor('#113'), QColor('#446'), QColor('#000')],
                #'button_focused_bg_colour': QColor(0, 0, 0x77),
            },
            'effect': {
                'bg_colours'       : [QColor('#543'), QColor('#876'), QColor('#210')],
                'fg_colour'        : QColor(0xff, 0xee, 0xdd),
                'button_bg_colours': [QColor('#543'), QColor('#876'), QColor('#210')],
                'button_down_bg_colours':
                                     [QColor('#321'), QColor('#654'), QColor('#000')],
                #'button_focused_bg_colour': QColor(0x77, 0x22, 0),
            },
            'proc_voice': {
                'bg_colours'       : [QColor('#255'), QColor('#588'), QColor('#022')],
                'fg_colour'        : QColor(0xcc, 0xff, 0xff),
                'button_bg_colours': [QColor('#255'), QColor('#588'), QColor('#022')],
                'button_down_bg_colours':
                                     [QColor('#033'), QColor('#366'), QColor('#000')],
                #'button_focused_bg_colour': QColor(0, 0x55, 0x55),
                'hilight_selected' : QColor(0x99, 0xbb, 0x99),
                'hilight_excluded' : QColor(0x55, 0x44, 0x33),
                'hilight_selected_focused': QColor(0xff, 0x88, 0x44),
                'hilight_excluded_focused': QColor(0x88, 0x33, 0x11),
                'hilight_pressed'  : QColor(0xff, 0xff, 0xff),
            },
            'proc_mixed': {
                'bg_colours'       : [QColor('#525'), QColor('#858'), QColor('#202')],
                'fg_colour'        : QColor(0xff, 0xcc, 0xff),
                'button_bg_colours': [QColor('#525'), QColor('#858'), QColor('#202')],
                'button_down_bg_colours':
                                     [QColor('#303'), QColor('#636'), QColor('#000')],
                #'button_focused_bg_colour': QColor(0x55, 0, 0x55),
                # TODO: Mixed processors shouldn't be highlighted;
                #       these are just a temp fix to prevent crash
                'hilight_selected' : QColor(0x99, 0xbb, 0x99),
                'hilight_excluded' : QColor(0x55, 0x44, 0x33),
                'hilight_selected_focused': QColor(0xff, 0x88, 0x44),
                'hilight_excluded_focused': QColor(0x88, 0x33, 0x11),
                'hilight_pressed'  : QColor(0xff, 0xff, 0xff),
            },
            'master': {
                'bg_colours'       : [QColor('#353'), QColor('#686'), QColor('#020')],
                'fg_colour'        : QColor(0xdd, 0xff, 0xdd),
                'button_bg_colours': [QColor('#353'), QColor('#686'), QColor('#020')],
                'button_down_bg_colours':
                                     [QColor('#131'), QColor('#464'), QColor('#000')],
                #'button_focused_bg_colour': QColor(0, 0x77, 0),
            },
        },
    }


# Vector utils needed for distance calculations

class Vec(tuple):

    def __new__(cls, coords):
        return tuple.__new__(cls, (float(x) for x in coords))

    def __add__(self, other):
        return Vec(x + y for (x, y) in zip(self, other))

    def __sub__(self, other):
        return Vec(x - y for (x, y) in zip(self, other))

    def __mul__(self, other):
        return Vec(x * other for x in self)

    def __rmul__(self, other):
        return self.__mul__(other)

def dot(a, b):
    return sum(x * y for (x, y) in zip(a, b))

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


class Connections(QAbstractScrollArea, AudioUnitUpdater):

    def __init__(self):
        super().__init__()

        self.setViewport(ConnectionsView())
        self.viewport().setFocusProxy(None)

        self.horizontalScrollBar().setSingleStep(8)
        self.verticalScrollBar().setSingleStep(8)

        self.add_to_updaters(self.viewport())

    def _on_setup(self):
        self.viewport().positionsChanged.connect(self._update_scrollbars)
        self._update_scrollbars()

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
        super().__init__(parent)
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


class ConnectionsView(QWidget, AudioUnitUpdater):

    positionsChanged = Signal(name='positionsChanged')

    def __init__(self):
        super().__init__()

        self._state = STATE_IDLE

        self._visible_device_ids = []
        self._visible_devices = {}

        self._centre_pos = (0, 0)

        self._focused_id = None
        self._focused_rel_pos = (0, 0)

        self._focused_button_info = {}
        self._pressed_button_info = {}

        self._focused_port_info = {}
        self._adding_edge_info = {}

        self._focused_edge_info = {}

        self._default_offsets = {}

        self._cable_cache = {}

        self._edit_dev_highlights = {}
        self._pressed_dev_id = None

        self._edge_menu = EdgeMenu(self)
        self._edge_menu.aboutToHide.connect(self._edge_menu_closing)
        self._edge_menu.triggered.connect(self._remove_edge)

        self._config = None

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

        self.setFocusPolicy(Qt.ClickFocus)
        self.setMouseTracking(True)

    def _on_setup(self):
        device_update_signals = [
            'signal_module',
            self._get_signal('signal_connections'),
            'signal_controls',
        ]
        if self._au_id != None:
            device_update_signals.extend([
                self._get_signal('signal_au_conns_hit'),
                self._get_signal('signal_au_conns_expr'),
                self._get_signal('signal_au_conns_edit_mode'),
                ])

        for signal in device_update_signals:
            self.register_action(signal, self._update_devices)

        self.register_action('signal_style_changed', self._update_style)

        self._update_style()
        self._update_devices()

    def get_area_rect(self):
        area_rect = None
        for device in self._visible_devices.values():
            dev_rect = device.get_rect_in_area()
            if not area_rect:
                area_rect = dev_rect
            else:
                area_rect = area_rect.united(dev_rect)
        return area_rect

    def _get_scaled_centre(self):
        style_mgr = self._ui_model.get_style_manager()
        return tuple(style_mgr.get_scaled_size(c, float('-inf'))
                for c in self._centre_pos)

    def get_visible_rect(self):
        scaled_centre = self._get_scaled_centre()
        x_start = scaled_centre[0] - self.width() // 2
        y_start = scaled_centre[1] - self.height() // 2
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

    def _change_layout_entry(self, key, value, mark_modified=True):
        connections = self._get_connections()
        layout = connections.get_layout()

        # Set new entry
        layout[key] = value

        # Also set entries for default offsets that were not changed
        for dev_id, offset in self._default_offsets.items():
            if (dev_id not in layout) or ('offset' not in layout[dev_id]):
                dev_layout = layout.get(dev_id, {})
                dev_layout['offset'] = offset
                layout[dev_id] = dev_layout
        self._default_offsets = {}

        connections.set_layout(layout, mark_modified)
        self._updater.signal_update(self._get_signal('signal_connections'))

    def scroll_to(self, area_x, area_y):
        area_rect = self.get_area_rect()
        visible_rect = self.get_visible_rect()
        if not area_rect or visible_rect.contains(area_rect, True):
            return

        style_mgr = self._ui_model.get_style_manager()
        inv_scale = 1 / style_mgr.get_scaled_size(1)

        full_rect = area_rect.united(visible_rect)
        new_centre_pos_px = (
                full_rect.left() + self.width() // 2 + area_x,
                full_rect.top() + self.height() // 2 + area_y)

        new_centre_pos = (new_centre_pos_px[0] * inv_scale,
                new_centre_pos_px[1] * inv_scale)

        self._change_layout_entry('centre_pos', new_centre_pos)

    def _set_config(self, config):
        self._config = DEFAULT_CONFIG.copy()

        if 'devices' in config:
            devices = config.pop('devices')
            self._config['devices'] = DEFAULT_CONFIG['devices'].copy()

            for dtype in ('instrument', 'effect', 'proc_voice', 'proc_mixed', 'master'):
                if dtype in devices:
                    self._config['devices'][dtype] = DEFAULT_CONFIG['devices'][dtype].copy()
                    self._config['devices'][dtype].update(devices.pop(dtype))
            self._config['devices'].update(devices)

        self._config.update(config)

        for device in self._visible_devices.values():
            device.set_config(self._config['devices'])
            device.draw_images()

        self._cable_cache = {}

        self._update_devices()

    def _update_style(self):
        style_mgr = self._ui_model.get_style_manager()

        def get_colour(name):
            return QColor(style_mgr.get_style_param(name))

        border_contrast = style_mgr.get_style_param('border_contrast')
        button_brightness = style_mgr.get_style_param('button_brightness')
        press_brightness = style_mgr.get_style_param('button_press_brightness')

        def get_outset_colours(name):
            return (QColor(style_mgr.get_adjusted_colour(name, 0)),
                    QColor(style_mgr.get_adjusted_colour(name, border_contrast)),
                    QColor(style_mgr.get_adjusted_colour(name, -border_contrast)))

        def get_button_colours(name):
            return (QColor(style_mgr.get_adjusted_colour(name, button_brightness)),
                    QColor(style_mgr.get_adjusted_colour(
                        name, button_brightness + border_contrast)),
                    QColor(style_mgr.get_adjusted_colour(
                        name, button_brightness - border_contrast)))

        def get_down_colours(name):
            down_brightness = button_brightness + press_brightness
            return (QColor(style_mgr.get_adjusted_colour(name, down_brightness)),
                    QColor(style_mgr.get_adjusted_colour(
                        name, down_brightness + border_contrast)),
                    QColor(style_mgr.get_adjusted_colour(
                        name, down_brightness - border_contrast)))

        pv_hilight_selected = get_colour('conns_proc_voice_selected_colour')
        focus_colour = get_colour('conns_focus_colour')
        bg_colour = get_colour('conns_bg_colour')

        focus_pressed_colour = QColor(style_mgr.get_adjusted_colour(
            'conns_focus_colour', press_brightness))

        pv_hilight_excluded = utils.lerp_colour(pv_hilight_selected, bg_colour, 0.5)
        pv_hilight_excluded_focused = utils.lerp_colour(focus_colour, bg_colour, 0.5)

        title_font = utils.get_scaled_font(style_mgr, 0.9, QFont.Bold)
        utils.set_glyph_rel_width(title_font, QWidget, string.ascii_lowercase, 14)
        port_font = utils.get_scaled_font(style_mgr, 0.7, QFont.Bold)

        devices = {
            'width'              : style_mgr.get_scaled_size(10.5),
            'title_font'         : title_font,
            'port_font'          : port_font,
            'port_handle_size'   : style_mgr.get_scaled_size(0.55),
            'port_focus_dist_max': style_mgr.get_scaled_size(0.55, 4),
            'port_colour'        : get_colour('conns_port_colour'),
            'focused_port_colour': focus_colour,
            'border_width'       : style_mgr.get_scaled_size_param('border_thin_width'),
            'border_radius'      : style_mgr.get_scaled_size_param('border_thin_radius'),
            'padding'            : style_mgr.get_scaled_size_param('medium_padding'),
            'button_width'       : style_mgr.get_scaled_size(4.5),
            'button_padding'     : style_mgr.get_scaled_size_param('small_padding'),
            'hilight_border'     : style_mgr.get_scaled_size(0.2),
            'hilight_padding'    : style_mgr.get_scaled_size(0.2),
            'instrument': {
                'bg_colours': get_outset_colours('conns_inst_bg_colour'),
                'fg_colour': get_colour('conns_inst_fg_colour'),
                'button_bg_colours': get_button_colours('conns_inst_bg_colour'),
                'button_down_bg_colours': get_down_colours('conns_inst_bg_colour'),
            },
            'effect': {
                'bg_colours': get_outset_colours('conns_effect_bg_colour'),
                'fg_colour': get_colour('conns_effect_fg_colour'),
                'button_bg_colours': get_button_colours('conns_effect_bg_colour'),
                'button_down_bg_colours': get_down_colours('conns_effect_bg_colour'),
            },
            'proc_voice': {
                'bg_colours': get_outset_colours('conns_proc_voice_bg_colour'),
                'fg_colour': get_colour('conns_proc_voice_fg_colour'),
                'button_bg_colours': get_button_colours('conns_proc_voice_bg_colour'),
                'button_down_bg_colours': get_down_colours('conns_proc_voice_bg_colour'),
                'hilight_selected': pv_hilight_selected,
                'hilight_excluded': pv_hilight_excluded,
                'hilight_selected_focused': focus_colour,
                'hilight_excluded_focused': pv_hilight_excluded_focused,
                'hilight_pressed' : focus_pressed_colour,
            },
            'proc_mixed': {
                'bg_colours': get_outset_colours('conns_proc_mixed_bg_colour'),
                'fg_colour': get_colour('conns_proc_mixed_fg_colour'),
                'button_bg_colours': get_button_colours('conns_proc_mixed_bg_colour'),
                'button_down_bg_colours': get_down_colours('conns_proc_mixed_bg_colour'),
            },
            'master': {
                'bg_colours': get_outset_colours('conns_master_bg_colour'),
                'fg_colour': get_colour('conns_master_fg_colour'),
            },
        }

        config = {
            'bg_colour':
                QColor(style_mgr.get_style_param('conns_bg_colour')),
            'edge_colour':
                QColor(style_mgr.get_style_param('conns_edge_colour')),
            'edge_width': style_mgr.get_scaled_size(0.1),
            'focused_edge_colour':
                QColor(style_mgr.get_style_param('conns_focus_colour')),
            'invalid_port_colour':
                QColor(style_mgr.get_style_param('conns_invalid_port_colour')),
            'focused_edge_width'        : style_mgr.get_scaled_size(0.3),
            'edge_focus_dist_max'       : style_mgr.get_scaled_size(0.8, 4),
            'invalid_port_line_width'   : style_mgr.get_scaled_size(0.3),
            'invalid_port_marker_size'  : style_mgr.get_scaled_size(1.2),

            'devices': devices,
        }

        self._set_config(config)
        self.update()

    def _get_full_id(self, dev_id):
        assert '/' not in dev_id
        if not self._au_id:
            return dev_id
        return '/'.join((self._au_id, dev_id))

    def _get_sub_id(self, full_dev_id):
        if not self._au_id:
            assert '/' not in full_dev_id
            return full_dev_id

        assert full_dev_id.startswith(self._au_id + '/')
        return full_dev_id[len(self._au_id) + 1:]

    def _get_device(self, dev_id):
        full_dev_id = self._get_full_id(dev_id)
        container = self._ui_model.get_module()
        if self._au_id != None:
            container = container.get_audio_unit(self._au_id)

        if dev_id.startswith('au'):
            return container.get_audio_unit(full_dev_id)
        elif dev_id.startswith('proc'):
            return container.get_processor(full_dev_id)

        return container

    def _get_in_ports(self, dev_id):
        device = self._get_device(dev_id)
        return device.get_in_ports()

    def _get_out_ports(self, dev_id):
        device = self._get_device(dev_id)
        return device.get_out_ports()

    def _get_port_info(self, dev_id):
        device = self._get_device(dev_id)
        return device.get_port_info()

    def _get_device_name(self, dev_id):
        device = self._get_device(dev_id)
        return device.get_name()

    def _update_devices(self):
        connections = self._get_connections()
        layout = connections.get_layout()

        self._centre_pos = layout.get('centre_pos', (0, 0))

        # Get visible device IDs
        visible_set = set(['master'])

        module = self._ui_model.get_module()
        if self._au_id != None:
            visible_set |= set(['Iin'])

            au = module.get_audio_unit(self._au_id)
            full_proc_ids = au.get_processor_ids()
            existent_proc_ids = [self._get_sub_id(fpid) for fpid in full_proc_ids
                    if au.get_processor(fpid).get_existence()]
            visible_set |= set(existent_proc_ids)

            full_eff_ids = au.get_au_ids()
            existent_eff_ids = [self._get_sub_id(feid) for feid in full_eff_ids
                    if au.get_audio_unit(feid).get_existence()]
            visible_set |= set(existent_eff_ids)

        else:
            full_au_ids = module.get_au_ids()
            existent_au_ids = [self._get_sub_id(faid) for faid in full_au_ids
                    if module.get_audio_unit(faid).get_existence()]
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

        # Remove outdated devices
        new_devices = {}
        for dev_id in self._visible_device_ids:
            if dev_id in self._visible_devices:
                old_device = self._visible_devices[dev_id]

                old_type_cfg_name = old_device.get_type_config_name()
                was_proc_voice = (old_type_cfg_name == 'proc_voice')

                model_device = self._get_device(dev_id)
                is_proc_voice = (isinstance(model_device, Processor) and
                        model_device.get_signal_type() == 'voice')

                if (old_device.get_name() == self._get_device_name(dev_id) and
                        (old_device.get_port_info() == self._get_port_info(dev_id)) and
                        was_proc_voice == is_proc_voice):
                    new_devices[dev_id] = old_device
        self._visible_devices = new_devices

        # Make new devices
        style_mgr = self._ui_model.get_style_manager()
        offset_x_abs = 18
        offset_y_abs = 10
        mid_offset = -offset_x_abs if (self._au_id == None) else 0
        default_pos_cfg = {
            'au':    { 'index': 0, 'offset_x': mid_offset,   'offset_y': offset_y_abs },
            'proc':  { 'index': 0, 'offset_x': mid_offset,   'offset_y': offset_y_abs },
            'eff':   { 'index': 0, 'offset_x': 0,            'offset_y': offset_y_abs },
            'master':{ 'index': 0, 'offset_x': offset_x_abs, 'offset_y': offset_y_abs },
            'Iin':   { 'index': 0, 'offset_x': -offset_x_abs, 'offset_y': offset_y_abs },
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
                        in_ports,
                        out_ports,
                        lambda: self._get_device(dev_id))
                device.set_config(self._config['devices'])
                device.draw_images()
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
                        self._centre_pos[0] + pos_cfg['offset_x'],
                        self._centre_pos[1] + y_offset_factor * pos_cfg['offset_y'])
                self._default_offsets[dev_id] = offset

            device = new_visible_devices[dev_id]

            offset_scaled = [style_mgr.get_scaled_size(o, float('-inf')) for o in offset]
            device.set_offset(offset_scaled)

        self._visible_devices = new_visible_devices

        # Get edit highlights
        self._edit_dev_highlights = {}
        if self._au_id != None:
            au = module.get_audio_unit(self._au_id)
            excluded = None

            # Find processor filter if appropriate
            conns_edit_mode = au.get_connections_edit_mode()
            if (conns_edit_mode == 'hit_proc_filter') and au.has_hits():
                hit_index = au.get_connections_hit_index()
                if hit_index != None:
                    hit = au.get_hit(hit_index)
                    if hit.get_existence():
                        excluded = hit.get_excluded_processors()
            elif (conns_edit_mode == 'expr_filter') and au.has_expressions():
                selected_expr = au.get_connections_expr_name()
                if au.has_expression(selected_expr):
                    excluded = au.get_expression_proc_filter(selected_expr)

            if excluded != None:
                visible_procs = set(dev_id for dev_id in self._visible_devices
                        if dev_id.startswith('proc'))
                for dev_id in visible_procs:
                    self._edit_dev_highlights[dev_id] = ('hilight_excluded'
                            if dev_id in excluded else 'hilight_selected')

        # Update finished
        self.positionsChanged.emit()
        self.update()

    def _split_path(self, path):
        parts = path.split('/')
        port_id = parts[-1]
        if len(parts) > 1:
            dev_id = parts[0]
        else:
            dev_id = 'master' if port_id.startswith('out') else 'Iin'
        return (dev_id, port_id)

    def _get_port_centre_from_path(self, path):
        dev_id, port_id = self._split_path(path)
        port_centre = self._visible_devices[dev_id].get_port_centre(port_id)
        return port_centre

    def paintEvent(self, event):
        start = time.time()

        painter = QPainter(self)
        painter.setBackground(self._config['bg_colour'])
        painter.eraseRect(0, 0, self.width(), self.height())

        scaled_centre = self._get_scaled_centre()

        painter.translate(
                self.width() // 2 - scaled_centre[0],
                self.height() // 2 - scaled_centre[1])

        connections = self._get_connections()
        layout = connections.get_layout()

        # Draw connections
        new_cable_cache = {}

        edges = connections.get_connections()
        for edge in edges:
            from_path, to_path = edge
            from_pos = self._get_port_centre_from_path(from_path)
            to_pos = self._get_port_centre_from_path(to_path)
            key = (from_pos, to_pos)
            if key in self._cable_cache:
                new_cable_cache[key] = self._cable_cache[key]
            else:
                cable = ConnectionCable(from_pos, to_pos)
                cable.set_colour(self._config['edge_colour'])
                cable.set_width(self._config['edge_width'])
                cable.set_focus_dist(self._config['edge_focus_dist_max'])
                cable.make_cable()
                new_cable_cache[key] = cable

        painter.save()
        painter.translate(0.5, 0.5)

        self._cable_cache = new_cable_cache
        for cable in self._cable_cache.values():
            cable.copy_cable(painter)
            #cable.debug_show_focus_map(painter)

        # Highlight focused connection
        if self._focused_edge_info:
            from_path, to_path = self._focused_edge_info['paths']
            from_pos = self._get_port_centre_from_path(from_path)
            to_pos = self._get_port_centre_from_path(to_path)

            pen = QPen(self._config['focused_edge_colour'])
            pen.setWidthF(self._config['focused_edge_width'])
            painter.setPen(pen)

            cable = ConnectionCable(from_pos, to_pos)
            cable.draw_cable(painter)

        painter.restore()

        # Draw connection that is being added
        if self._adding_edge_info:
            from_info = self._adding_edge_info['from']
            from_dev_id = from_info['dev_id']
            from_port = from_info['port']
            from_pos = self._visible_devices[from_dev_id].get_port_centre(from_port)
            from_x, from_y = from_pos

            pen = QPen(self._config['focused_edge_colour'])
            to_info = self._adding_edge_info['to']

            if to_info:
                to_dev_id = to_info['dev_id']
                to_port = to_info['port']
                to_pos = self._visible_devices[to_dev_id].get_port_centre(to_port)
                to_x, to_y = to_pos
                pen.setWidthF(self._config['focused_edge_width'])
            else:
                to_pos = self._adding_edge_info['mouse_pos']
                to_x, to_y = to_pos
                pen.setWidthF(self._config['edge_width'])

            painter.save()
            painter.translate(0.5, 0.5)
            painter.setPen(pen)
            painter.setRenderHint(QPainter.Antialiasing)
            vis_from_pos, vis_to_pos = from_pos, to_pos
            if not self._is_send_port(from_dev_id, from_port):
                vis_from_pos, vis_to_pos = to_pos, from_pos
            cable = ConnectionCable(vis_from_pos, vis_to_pos)
            cable.draw_cable(painter)
            painter.restore()

        # Draw devices
        for dev_id in self._visible_device_ids:
            # Verify existence of the device
            # This fixes the occasional momentary glitch after device removal
            model_device = self._get_device(dev_id)
            if not isinstance(model_device, Module):
                if not model_device.get_existence():
                    continue

            device = self._visible_devices[dev_id]
            device.copy_images(painter)

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

            # Draw device highlight
            if dev_id in self._edit_dev_highlights:
                highlight = self._edit_dev_highlights[dev_id]
                if dev_id == self._focused_id:
                    if dev_id == self._pressed_dev_id:
                        highlight = 'hilight_pressed'
                    else:
                        if highlight == 'hilight_selected':
                            highlight = 'hilight_selected_focused'
                        else:
                            highlight = 'hilight_excluded_focused'
                device.draw_device_highlight(painter, highlight)

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
            pos = self._visible_devices[dev_id].get_port_centre(port_id)
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
        scaled_centre = self._get_scaled_centre()
        return (widget_x - (self.width() // 2) + scaled_centre[0],
                widget_y - (self.height() // 2) + scaled_centre[1])

    def _edge_menu_closing(self):
        self._state = STATE_IDLE
        self._focused_edge_info = {}

    def _remove_edge(self, action):
        edge = self._edge_menu.get_edge()
        connections = self._get_connections()
        edges = connections.get_connections()
        edges.remove(edge)
        connections.set_connections(edges)
        self._updater.signal_update(self._get_signal('signal_connections'))
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

    def _get_connections_edit_mode(self):
        mode = 'normal'
        if self._au_id != None:
            module = self._ui_model.get_module()
            au = module.get_audio_unit(self._au_id)
            mode = au.get_connections_edit_mode()
        return mode

    def _handle_mouse_move_normal(self, event):
        area_pos = self._get_area_pos(event.x(), event.y())

        new_focused_port_info = {}
        new_focused_button_info = {}
        new_focused_edge_info = {}

        style_mgr = self._ui_model.get_style_manager()

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
                        new_focused_button_info = {
                            'dev_id': dev_id,
                            'button_type': device.get_button_type_at(dev_rel_pos),
                        }
                    break

            if not on_device:
                # Find a focused edge
                connections = self._get_connections()
                edges = connections.get_connections()
                for edge in edges:
                    from_path, to_path = edge
                    from_pos = self._get_port_centre_from_path(from_path)
                    to_pos = self._get_port_centre_from_path(to_path)
                    key = (from_pos, to_pos)
                    if key in self._cable_cache:
                        cable = self._cable_cache[key]
                    else:
                        print('Missing cable from cache')
                        cable = ConnectionCable(from_pos, to_pos)
                        cable.set_focus_dist(self._config['edge_focus_dist_max'])
                        cable.make_cable()
                    if cable.is_near_point(area_pos):
                        new_focused_edge_info = { 'paths': edge }

        elif self._state == STATE_MOVING:
            if (not self._focused_id) or (self._focused_id not in self._visible_devices):
                self._state = STATE_IDLE
            else:
                # Move focused device
                area_x, area_y = area_pos
                new_offset_x_px = area_x - self._focused_rel_pos[0]
                new_offset_y_px = area_y - self._focused_rel_pos[1]

                inv_scale = 1 / style_mgr.get_scaled_size(1)
                new_offset_x = new_offset_x_px * inv_scale
                new_offset_y = new_offset_y_px * inv_scale

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
                            'button_type': device.get_button_type_at(dev_rel_pos),
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
                            not self._edge_completes_cycle(from_info, to_info)):
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

    def _handle_mouse_move_proc_filters(self, event):
        area_pos = self._get_area_pos(event.x(), event.y())

        if self._state in (STATE_IDLE, STATE_PRESSING):
            prev_focused_id = self._focused_id

            for dev_id in reversed(self._visible_device_ids):
                if not dev_id.startswith('proc'):
                    continue

                device = self._visible_devices[dev_id]
                dev_rel_pos = device.get_rel_pos(area_pos)
                if device.contains_rel_pos(dev_rel_pos):
                    if self._state == STATE_IDLE:
                        self._focused_id = dev_id
                    elif self._state == STATE_PRESSING:
                        if dev_id == self._pressed_dev_id:
                            self._focused_id = dev_id
                        else:
                            self._focused_id = None
                    break
            else:
                self._focused_id = None

            if self._focused_id != prev_focused_id:
                self.update()

    def mouseMoveEvent(self, event):
        mode = self._get_connections_edit_mode()
        if mode == 'normal':
            self._handle_mouse_move_normal(event)
        elif mode in ('hit_proc_filter', 'expr_filter'):
            self._handle_mouse_move_proc_filters(event)
        else:
            assert False

    def _is_device_behind_another(self, check_dev_id):
        connections = self._get_connections()
        layout = connections.get_layout()
        z_order = layout.get('z_order', [])

        def get_z_index(dev_id):
            return z_order.index(dev_id) if dev_id in z_order else -1

        check_device = self._visible_devices[check_dev_id]
        check_rect = check_device.get_rect_in_area()
        for dev_id in self._visible_device_ids:
            if dev_id == check_dev_id:
                continue
            device = self._visible_devices[dev_id]
            if (device.get_rect_in_area().intersects(check_rect) and
                    get_z_index(check_dev_id) <= get_z_index(dev_id)):
                return True

        return False

    def _handle_mouse_press_normal(self, event):
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
            new_visible_ids = list(self._visible_device_ids)
            new_visible_ids.remove(self._focused_id)
            new_visible_ids.append(self._focused_id)

            mark_modified = self._is_device_behind_another(self._focused_id)

            self._change_layout_entry('z_order', new_visible_ids, mark_modified)

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
                    'dev_id': self._focused_id,
                    'button_type': device.get_button_type_at(dev_rel_pos),
                    'pressed': True
                }
                self._focused_button_info = self._pressed_button_info.copy()
            elif self._focused_port_info:
                self._state = STATE_ADDING_EDGE
                self._adding_edge_info = {
                    'from': self._focused_port_info, 'mouse_pos': area_pos, 'to': {},
                }
            else:
                self._state = STATE_MOVING

    def _handle_mouse_press_proc_filters(self, event):
        area_pos = self._get_area_pos(event.x(), event.y())

        for dev_id in reversed(self._visible_device_ids):
            if not dev_id.startswith('proc'):
                continue

            device = self._visible_devices[dev_id]
            dev_rel_pos = device.get_rel_pos(area_pos)
            if device.contains_rel_pos(dev_rel_pos):
                self._pressed_dev_id = dev_id
                self._state = STATE_PRESSING
                self.update()
                break
        else:
            self._pressed_dev_id = None

    def mousePressEvent(self, event):
        mode = self._get_connections_edit_mode()
        if mode == 'normal':
            self._handle_mouse_press_normal(event)
        elif mode in ('hit_proc_filter', 'expr_filter'):
            self._handle_mouse_press_proc_filters(event)
        else:
            assert False

    def _handle_mouse_release_normal(self, event):
        if self._state == STATE_EDGE_MENU:
            return

        self._focused_id = None
        self._focused_rel_pos = (0, 0)

        if self._state == STATE_MOVING:
            pass
        elif self._state == STATE_PRESSING:
            shift_pressed = (event.modifiers() == Qt.ShiftModifier)
            clicked = (self._focused_button_info == self._pressed_button_info)
            if clicked:
                self._perform_button_click(self._focused_button_info, shift_pressed)

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
            self._updater.signal_update(self._get_signal('signal_connections'))
            self.update()

        self._state = STATE_IDLE

    def _handle_mouse_release_proc_filters(self, event, mode):
        if self._state == STATE_PRESSING:
            assert self._pressed_dev_id.startswith('proc')
            if self._focused_id == self._pressed_dev_id:
                module = self._ui_model.get_module()
                au = module.get_audio_unit(self._au_id)

                if mode == 'hit_proc_filter':
                    # Get current hit
                    hit_index = au.get_connections_hit_index()
                    hit = au.get_hit(hit_index)

                    # Toggle processor usage in hit processing
                    excluded = hit.get_excluded_processors()
                    if self._focused_id in excluded:
                        excluded.remove(self._focused_id)
                    else:
                        excluded.append(self._focused_id)
                    hit.set_excluded_processors(excluded)
                    self._updater.signal_update(
                            self._get_signal('signal_au_conns_hit'))

                elif mode == 'expr_filter':
                    # Update exclusion list
                    selected_expr_name = au.get_connections_expr_name()
                    excluded = au.get_expression_proc_filter(selected_expr_name)
                    if self._focused_id in excluded:
                        excluded.remove(self._focused_id)
                    else:
                        excluded.append(self._focused_id)
                    au.set_expression_proc_filter(selected_expr_name, excluded)
                    self._updater.signal_update(
                            self._get_signal('signal_au_conns_expr'))

                else:
                    assert False


        self._pressed_dev_id = None
        self.update()

        self._state = STATE_IDLE

    def mouseReleaseEvent(self, event):
        mode = self._get_connections_edit_mode()
        if mode == 'normal':
            self._handle_mouse_release_normal(event)
        elif mode in ('hit_proc_filter', 'expr_filter'):
            self._handle_mouse_release_proc_filters(event, mode)
        else:
            assert False

    def _perform_button_click(self, button_info, shift_pressed):
        visibility_mgr = self._ui_model.get_visibility_manager()
        dev_id = button_info['dev_id']
        if dev_id.startswith('au'):
            if button_info['button_type'] == 'edit':
                visibility_mgr.show_audio_unit(self._get_full_id(dev_id))
            elif (button_info['button_type'] == 'remove'):
                remove_action = lambda: self._remove_au(dev_id)
                if shift_pressed:
                    remove_action()
                else:
                    dialog = RemoveDeviceConfirmDialog(self._ui_model, remove_action)
                    dialog.exec_()
        elif dev_id.startswith('proc'):
            if button_info['button_type'] == 'edit':
                visibility_mgr.show_processor(self._get_full_id(dev_id))
            elif (button_info['button_type'] == 'remove'):
                remove_action = lambda: self._remove_proc(dev_id)
                if shift_pressed:
                    remove_action()
                else:
                    dialog = RemoveDeviceConfirmDialog(self._ui_model, remove_action)
                    dialog.exec_()

    def _remove_au(self, dev_id):
        full_dev_id = self._get_full_id(dev_id)

        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.hide_audio_unit_and_subdevices(full_dev_id)

        connections = self._get_connections()
        connections.disconnect_device(full_dev_id)

        module = self._ui_model.get_module()

        if self._au_id == None:
            module.remove_controls_to_audio_unit(full_dev_id)

        if self._au_id != None:
            container = module.get_audio_unit(self._au_id)
        else:
            container = module
        container.remove_audio_unit(full_dev_id)

        layout = connections.get_layout()
        if dev_id in layout:
            del layout[dev_id]
            connections.set_layout(layout)

        update_signals = [self._get_signal('signal_connections')]
        if self._au_id == None:
            update_signals.append('signal_controls')
        self._updater.signal_update(*update_signals)

    def _remove_proc(self, dev_id):
        full_dev_id = self._get_full_id(dev_id)

        assert self._au_id != None
        visibility_mgr = self._ui_model.get_visibility_manager()
        visibility_mgr.hide_processor(full_dev_id)

        connections = self._get_connections()
        connections.disconnect_device(full_dev_id)

        module = self._ui_model.get_module()
        au = module.get_audio_unit(self._au_id)
        au.remove_processor(full_dev_id)

        layout = connections.get_layout()
        if dev_id in layout:
            del layout[dev_id]
            connections.set_layout(layout)

        update_signals = [self._get_signal('signal_connections')]
        self._updater.signal_update(*update_signals)

    def leaveEvent(self, event):
        if self._state == STATE_EDGE_MENU:
            return

        self._focused_id = None
        self._focused_rel_pos = (0, 0)
        self._focused_button_info = {}
        self._pressed_button_info = {}
        self._focused_port_info = {}
        self._focused_edge_info = {}


class RemoveDeviceConfirmDialog(ConfirmDialog):

    def __init__(self, ui_model, action_on_confirm):
        super().__init__(ui_model)

        self._action_on_confirm = action_on_confirm

        self.setWindowTitle('Confirm device removal')

        msg = '<p>Do you really want to remove the device?</p>'
        msg += ('<p>(Tip: You can Shift+click the Del button to remove'
            ' without confirmation.)</p>')
        self._set_message(msg)

        self._cancel_button = QPushButton('Keep the device')
        self._remove_button = QPushButton('Remove the device')

        b = self._get_button_layout()
        b.addWidget(self._cancel_button)
        b.addWidget(self._remove_button)

        self._cancel_button.clicked.connect(self.close)
        self._remove_button.clicked.connect(self._perform_action)

    def _perform_action(self):
        self._action_on_confirm()
        self.close()


class Device():

    def __init__(self, dev_id, in_ports, out_ports, get_model_device):
        self._id = dev_id
        self._config = None
        self._type_config = None

        self._offset_x = 0
        self._offset_y = 0

        model_device = get_model_device()
        name = model_device.get_name()

        if self._id in ('master', 'Iin'):
            self._type = 'master'
        elif self._id.startswith('au'):
            if model_device.is_instrument():
                self._type = 'instrument'
            elif model_device.is_effect():
                self._type = 'effect'
            else:
                assert False
        elif self._id.startswith('proc'):
            if model_device.get_signal_type() == 'voice':
                self._type = 'proc_voice'
            elif model_device.get_signal_type() == 'mixed':
                self._type = 'proc_mixed'
            else:
                assert False
        else:
            raise ValueError('Unexpected type of device ID: {}'.format(dev_id))

        self._name = name

        self._in_ports = in_ports
        self._out_ports = out_ports

        self._port_names = model_device.get_port_info()

        self._bg = None

    def set_config(self, config):
        self._config = config
        self._type_config = self._config[self._type]

    def get_name(self):
        return self._name

    def get_type_config_name(self):
        for key, v in self._config.items():
            if self._type_config == v:
                return key

    def get_port_info(self):
        return self._port_names

    def draw_images(self):
        margin = self._get_margin()
        self._bg = QImage(
                self._config['width'] + margin * 2,
                self._get_height(),
                QImage.Format_ARGB32_Premultiplied)
        self._bg.fill(0)
        painter = QPainter(self._bg)
        pad = self._config['padding']

        text_scale_mult = 4
        scaled_pad = text_scale_mult * pad

        # Background
        self._draw_rounded_rect(
                painter,
                self._type_config['bg_colours'],
                QRect(margin, margin,
                    self._bg.width() - margin, self._bg.height() - margin))

        # Title
        painter.setPen(QColor(self._type_config['fg_colour']))
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

        # Port texts
        port_font = QFont(self._config['port_font'])
        port_font.setPointSizeF(port_font.pointSizeF() * text_scale_mult)
        utils.set_glyph_rel_width(port_font, QWidget, string.ascii_lowercase, 14)
        port_height = self._get_port_height()

        def get_port_text_image(port_str, text_option):
            port_img = QImage(
                    text_scale_mult * self._bg.width() // 2,
                    text_scale_mult * port_height,
                    QImage.Format_ARGB32_Premultiplied)
            port_img.fill(0)

            img_painter = QPainter(port_img)
            img_painter.setPen(QColor(self._type_config['fg_colour']))
            img_painter.setFont(port_font)
            img_painter.drawText(
                    QRectF(
                        scaled_pad, 0,
                        port_img.width() - scaled_pad * 2, port_img.height()),
                    port_str,
                    text_option)
            img_painter.end()

            return port_img.scaledToHeight(port_height, Qt.SmoothTransformation)

        text_option = QTextOption(Qt.AlignLeft | Qt.AlignVCenter)
        port_y = self._get_title_height() + pad

        for port_id in self._in_ports:
            port_num = int(port_id.split('_')[1], 16)
            port_str = self._port_names.get(port_id, None) or str(port_num)

            port_img = get_port_text_image(port_str, text_option)
            painter.drawImage(
                    QPoint(0, port_y),
                    port_img,
                    port_img.rect())

            port_y += port_height

        text_option = QTextOption(Qt.AlignRight | Qt.AlignVCenter)
        port_y = self._get_title_height() + pad
        for port_id in self._out_ports:
            port_num = int(port_id.split('_')[1], 16)
            port_str = self._port_names.get(port_id, None) or str(port_num)

            port_img = get_port_text_image(port_str, text_option)
            painter.drawImage(
                    QPoint(self._bg.width() // 2, port_y),
                    port_img,
                    port_img.rect())

            port_y += port_height

        # Edit button
        if self._has_edit_button():
            self._draw_edit_button(
                    painter,
                    self._type_config['button_bg_colours'],
                    self._type_config['fg_colour'])

        # Remove button
        if self._has_remove_button():
            self._draw_remove_button(
                    painter,
                    self._type_config['button_bg_colours'],
                    self._type_config['fg_colour'])

    def copy_images(self, painter):
        painter.save()

        painter.translate(self._offset_x, self._offset_y)

        bg_offset_x, bg_offset_y = self._get_top_left_pos((0, 0))
        painter.drawImage(bg_offset_x, bg_offset_y, self._bg)

        painter.restore()

    def _get_in_port_centres(self):
        bg_offset_x, bg_offset_y = self._get_top_left_pos(
                (self._offset_x, self._offset_y))

        port_offset = self._config['port_handle_size'] // 2
        padding = self._config['padding']
        port_x = bg_offset_x - port_offset + self._get_margin()
        port_y = bg_offset_y + padding + self._get_title_height() + padding + port_offset

        for _ in self._in_ports:
            yield (port_x, port_y)
            port_y += self._get_port_height()

    def _get_out_port_centres(self):
        bg_offset_x, bg_offset_y = self._get_top_left_pos(
                (self._offset_x, self._offset_y))

        port_offset = self._config['port_handle_size'] // 2
        padding = self._config['padding']
        port_x = bg_offset_x + self._bg.width() + port_offset - self._get_margin() - 1
        port_y = bg_offset_y + padding + self._get_title_height() + padding + port_offset

        for _ in self._out_ports:
            yield (port_x, port_y)
            port_y += self._get_port_height()

    def get_port_centre(self, port_id):
        if port_id.startswith('in') != (self._id in ('master', 'Iin')):
            for i, point in enumerate(self._get_in_port_centres()):
                if self._in_ports[i] == port_id:
                    return point
        else:
            for i, point in enumerate(self._get_out_port_centres()):
                if self._out_ports[i] == port_id:
                    return point

        assert False, 'Device {} does not have port {}'.format(self._id, port_id)

    def _get_in_port_rects(self):
        handle_size = self._config['port_handle_size']
        handle_offset = -handle_size // 2 + 1
        for point in self._get_in_port_centres():
            x, y = point
            yield QRect(x + handle_offset, y + handle_offset, handle_size, handle_size)

    def _get_out_port_rects(self):
        handle_size = self._config['port_handle_size']
        handle_offset = -handle_size // 2 + 1
        for point in self._get_out_port_centres():
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

    def draw_device_highlight(self, painter, highlight_mode):
        assert self._id.startswith('proc')
        painter.save()

        extent = self._config['hilight_border'] + self._config['hilight_padding']

        colour = self._type_config[highlight_mode]
        pen = QPen(colour)
        pen.setWidth(self._config['hilight_border'])
        painter.setPen(pen)
        if 'excluded' in highlight_mode: # TODO: clean up
            painter.setBrush(QColor(0, 0, 0, 0x77))
        painter.setRenderHint(QPainter.Antialiasing)

        exbr = extent - 1
        rect = self.get_rect_in_area().adjusted(-extent, -extent, exbr, exbr)
        painter.drawRect(rect)

        painter.restore()

    def draw_button_highlight(self, painter, info):
        assert 'button_type' in info

        if not info.get('pressed'):
            return # TODO: add hover support if needed

        painter.save()
        shift_x, shift_y = self._get_top_left_pos((0, 0))
        painter.translate(self._offset_x + shift_x, self._offset_y + shift_y)

        bg_colours = self._type_config['button_down_bg_colours']
        fg_colour = self._type_config['fg_colour']

        if info['button_type'] == 'edit':
            assert self._has_edit_button()
            self._draw_edit_button(painter, bg_colours, fg_colour)
        elif info['button_type'] == 'remove':
            assert self._has_remove_button()
            self._draw_remove_button(painter, bg_colours, fg_colour)

        painter.restore()

    def _get_top_left_pos(self, centre_pos):
        centre_x, centre_y = centre_pos
        return (centre_x - self._bg.width() // 2, centre_y - self._bg.height() // 2)

    def _get_dev_biased_pos(self, centre_pos):
        centre_x, centre_y = centre_pos
        return (centre_x + self._bg.width() // 2, centre_y + self._bg.height() // 2)

    def _draw_rounded_rect(self, painter, colours, rect):
        inner_rect = rect.adjusted(1, 1, -1, -1)
        assert inner_rect.isValid()

        # Background fill
        painter.setPen(Qt.NoPen)
        painter.fillRect(inner_rect, colours[0])

        painter.save()
        painter.setRenderHint(QPainter.Antialiasing)
        top = rect.top() + 0.5
        left = rect.left() + 0.5
        right = rect.right() + 0.5
        bottom = rect.bottom() + 0.5

        width = self._config['border_width']

        radius = self._config['border_radius']
        diam = radius * 2

        # Dark shade
        painter.setPen(QPen(QBrush(colours[2]), width, cap=Qt.FlatCap))
        dark_path = QPainterPath()
        dark_path.arcMoveTo(QRectF(left, bottom - diam, diam, diam), 225)
        dark_path.arcTo(QRectF(left, bottom - diam, diam, diam), 225, 45)
        dark_path.lineTo(QPointF(right - radius, bottom))
        dark_path.arcTo(QRectF(right - diam, bottom - diam, diam, diam), 270, 90)
        dark_path.lineTo(QPointF(right, top + radius))
        dark_path.arcTo(QRectF(right - diam, top, diam, diam), 0, 45)
        painter.drawPath(dark_path)

        # Light shade
        painter.setPen(QPen(QBrush(colours[1]), width, cap=Qt.FlatCap))
        light_path = QPainterPath()
        light_path.arcMoveTo(QRectF(left, bottom - diam, diam, diam), 225)
        light_path.arcTo(QRectF(left, bottom - diam, diam, diam), 225, -45)
        light_path.lineTo(QPointF(left, top + radius))
        light_path.arcTo(QRectF(left, top, diam, diam), 180, -90)
        light_path.lineTo(QPointF(right - radius, top))
        light_path.arcTo(QRectF(right - diam, top, diam, diam), 90, -45)
        painter.drawPath(light_path)

        painter.restore()

    def _draw_button(self, painter, bg_colours, fg_colour, rect, text):
        self._draw_rounded_rect(painter, bg_colours, rect)

        painter.setPen(fg_colour)

        painter.setFont(self._config['title_font'])
        text_option = QTextOption(Qt.AlignCenter)
        painter.drawText(QRectF(rect), text, text_option)

    def _draw_edit_button(self, painter, bg_colours, fg_colour):
        rect = self._get_edit_button_rect()
        self._draw_button(painter, bg_colours, fg_colour, rect, 'Edit')

    def _draw_remove_button(self, painter, bg_colours, fg_colour):
        rect = self._get_remove_button_rect()
        self._draw_button(painter, bg_colours, fg_colour, rect, 'Del')

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
            for i, point in enumerate(self._get_in_port_centres()):
                x, y = point
                x -= self._offset_x
                y -= self._offset_y
                if max(abs(dev_x - x), abs(dev_y - y)) <= port_dist_max:
                    return self._in_ports[i]
        else:
            for i, point in enumerate(self._get_out_port_centres()):
                x, y = point
                x -= self._offset_x
                y -= self._offset_y
                if max(abs(dev_x - x), abs(dev_y - y)) <= port_dist_max:
                    return self._out_ports[i]

        return None

    def _has_edit_button_at(self, rel_pos):
        if not self._has_edit_button():
            return False

        biased_x, biased_y = self._get_dev_biased_pos(rel_pos)
        rect = self._get_edit_button_rect()
        return (rect.left() <= biased_x <= rect.right() and
                rect.top() <= biased_y <= rect.bottom())

    def _has_remove_button_at(self, rel_pos):
        if not self._has_remove_button():
            return False

        biased_x, biased_y = self._get_dev_biased_pos(rel_pos)
        rect = self._get_remove_button_rect()
        return (rect.left() <= biased_x <= rect.right() and
                rect.top() <= biased_y <= rect.bottom())

    def has_button_at(self, rel_pos):
        return (self._has_edit_button_at(rel_pos) or self._has_remove_button_at(rel_pos))

    def get_button_type_at(self, rel_pos):
        if self._has_edit_button_at(rel_pos):
            return 'edit'
        elif self._has_remove_button_at(rel_pos):
            return 'remove'
        else:
            assert False

    def get_rect_in_area(self):
        return QRect(
                self._offset_x - self._bg.width() // 2,
                self._offset_y - self._bg.height() // 2,
                self._bg.width() + 1,
                self._bg.height() + 1)

    def _has_edit_button(self):
        return ((self._id not in ('master', 'Iin')) and
                (self._id.startswith('au') or self._id.startswith('proc')))

    def _has_remove_button(self):
        return ((self._id not in ('master', 'Iin')) and
                (self._id.startswith('au') or self._id.startswith('proc')))

    def _get_margin(self):
        border_width = self._config['border_width']
        margin = int(math.ceil(border_width / 2))
        return margin

    def _get_width(self):
        return self._config['width'] + self._get_margin() * 2

    def _get_height(self):
        title_height = self._get_title_height()
        port_height = self._get_port_height()
        ports_height = max(len(self._in_ports), len(self._out_ports)) * port_height

        padding = self._config['padding']

        if ports_height > 0:
            total_height = title_height + padding + ports_height
        else:
            total_height = title_height + padding * 2

        if self._has_edit_button():
            edit_button_height = self._get_edit_button_height()
            total_height += edit_button_height + self._config['padding']

        total_height += self._get_margin()

        return total_height

    def _get_title_height(self):
        fm = QFontMetrics(self._config['title_font'])
        return fm.boundingRect('Ag').height() + self._get_margin()

    def _get_port_height(self):
        fm = QFontMetrics(self._config['port_font'])
        return self._config['padding'] + fm.boundingRect('0').height()

    def _get_edit_button_height(self):
        return self._get_title_height() + self._config['button_padding'] * 2

    def _get_edit_button_rect(self):
        height = self._get_edit_button_height()
        left = self._get_margin() + self._config['padding']
        top = self._get_height() - height - self._config['padding'] - self._get_margin()
        return QRect(
                left, top,
                self._config['button_width'] - 1, height - 1)

    def _get_remove_button_height(self):
        return self._get_title_height() + self._config['button_padding'] * 2

    def _get_remove_button_rect(self):
        width = self._config['button_width']
        height = self._get_remove_button_height()
        margin = self._get_margin()
        padding = self._config['padding']
        left = (self._get_width() - width) - margin - padding
        top = self._get_height() - height - padding - margin
        return QRect(left, top, width - 1, height - 1)


