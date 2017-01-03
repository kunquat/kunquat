# -*- coding: utf-8 -*-

#
# Author: Tomi Jylhä-Ollila, Finland 2016-2017
#
# This file is part of Kunquat.
#
# CC0 1.0 Universal, http://creativecommons.org/publicdomain/zero/1.0/
#
# To the extent possible under law, Kunquat Affirmers have waived all
# copyright and related or neighboring rights to Kunquat.
#

import math
import os
import string

from PySide.QtCore import *
from PySide.QtGui import *

import kunquat.tracker.config as config
from .headerline import HeaderLine
from . import utils


class Settings(QWidget):

    def __init__(self):
        super().__init__()

        self._modules = Modules()
        self._instruments = Instruments()
        self._samples = Samples()
        self._effects = Effects()

        self._style_toggle = StyleToggle()
        self._colours = Colours()

        dgl = QGridLayout()
        dgl.setContentsMargins(0, 0, 0, 0)
        dgl.setHorizontalSpacing(4)
        dgl.setVerticalSpacing(2)
        dgl.addWidget(QLabel('Modules:'), 0, 0)
        dgl.addWidget(self._modules, 0, 1)
        dgl.addWidget(QLabel('Instruments:'), 1, 0)
        dgl.addWidget(self._instruments, 1, 1)
        dgl.addWidget(QLabel('Samples:'), 2, 0)
        dgl.addWidget(self._samples, 2, 1)
        dgl.addWidget(QLabel('Effects:'), 3, 0)
        dgl.addWidget(self._effects, 3, 1)

        dl = QVBoxLayout()
        dl.setContentsMargins(0, 0, 0, 0)
        dl.setSpacing(4)
        dl.addWidget(HeaderLine('Default directories'))
        dl.addLayout(dgl)
        dl.addStretch(1)

        uil = QVBoxLayout()
        uil.setContentsMargins(0, 0, 0, 0)
        uil.setSpacing(4)
        uil.addWidget(HeaderLine('User interface'))
        uil.addWidget(self._style_toggle)
        uil.addWidget(self._colours)

        h = QHBoxLayout()
        h.setContentsMargins(2, 2, 2, 2)
        h.setSpacing(8)
        h.addLayout(dl)
        h.addLayout(uil)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._modules.set_ui_model(ui_model)
        self._instruments.set_ui_model(ui_model)
        self._samples.set_ui_model(ui_model)
        self._effects.set_ui_model(ui_model)
        self._style_toggle.set_ui_model(ui_model)
        self._colours.set_ui_model(ui_model)

    def unregister_updaters(self):
        self._colours.unregister_updaters()
        self._style_toggle.unregister_updaters()
        self._effects.unregister_updaters()
        self._samples.unregister_updaters()
        self._instruments.unregister_updaters()
        self._modules.unregister_updaters()


class Directory(QWidget):

    def __init__(self, conf_key):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self._conf_key = conf_key

        self._text = QLineEdit()
        self._browse = QPushButton('Browse...')

        h = QHBoxLayout()
        h.setContentsMargins(0, 0, 0, 0)
        h.setSpacing(4)
        h.addWidget(self._text)
        h.addWidget(self._browse)
        self.setLayout(h)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(
                self._text, SIGNAL('textEdited(const QString&)'), self._change_dir_text)
        QObject.connect(self._browse, SIGNAL('clicked()'), self._change_dir_browse)

        self._update_dir()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_settings_dir' in signals:
            self._update_dir()

    def _update_dir(self):
        cfg = config.get_config()
        directory = cfg.get_value(self._conf_key) or ''

        old_block = self._text.blockSignals(True)
        if directory != self._text.text():
            self._text.setText(directory)
        self._text.blockSignals(old_block)

    def _change_dir_text(self, text):
        cfg = config.get_config()
        cfg.set_value(self._conf_key, text)
        self._updater.signal_update(set(['signal_settings_dir']))

    def _change_dir_browse(self):
        cfg = config.get_config()
        cur_dir = cfg.get_value(self._conf_key) or os.getcwd()
        new_dir = QFileDialog.getExistingDirectory(dir=cur_dir)
        if new_dir:
            self._change_dir_text(new_dir)


class Modules(Directory):

    def __init__(self):
        super().__init__('dir_modules')


class Instruments(Directory):

    def __init__(self):
        super().__init__('dir_instruments')


class Samples(Directory):

    def __init__(self):
        super().__init__('dir_samples')


class Effects(Directory):

    def __init__(self):
        super().__init__('dir_effects')


class StyleToggle(QCheckBox):

    def __init__(self):
        super().__init__('Enable custom style')
        self._ui_model = None
        self._updater = None

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        QObject.connect(self, SIGNAL('stateChanged(int)'), self._change_enabled)

        self._update_enabled()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_style_changed' in signals:
            self._update_enabled()

    def _update_enabled(self):
        style_manager = self._ui_model.get_style_manager()
        enabled = style_manager.is_custom_style_enabled()

        old_block = self.blockSignals(True)
        self.setCheckState(Qt.Checked if enabled else Qt.Unchecked)
        self.blockSignals(old_block)

    def _change_enabled(self, state):
        enabled = (state == Qt.Checked)

        style_manager = self._ui_model.get_style_manager()
        style_manager.set_custom_style_enabled(enabled)

        self._updater.signal_update(set(['signal_style_changed']))


class ColourCategoryModel():

    def __init__(self, name):
        self._name = name
        self._colours = []

    def add_colour(self, colour):
        self._colours.append(colour)

    def get_name(self):
        return self._name

    def get_colour_count(self):
        return len(self._colours)

    def get_colour(self, index):
        return self._colours[index]

    def get_index_of_colour(self, colour):
        return self._colours.index(colour)


class ColourModel():

    def __init__(self, key, colour, category=None):
        self._key = key
        self._colour = colour
        self._category = category

    def get_key(self):
        return self._key

    def get_colour(self):
        return self._colour

    def get_category(self):
        return self._category


_COLOUR_DESCS = [
    ('bg_colour',                       'Default background'),
    ('fg_colour',                       'Default text'),
    ('disabled_fg_colour',              'Disabled text'),
    ('button_bg_colour',                'Button background'),
    ('button_fg_colour',                'Button foreground'),
    ('text_bg_colour',                  'Text field background'),
    ('text_fg_colour',                  'Text field foreground'),
    ('sheet_area_selection_colour',     'Sheet area selection'),
    ('sheet_canvas_bg_colour',          'Sheet canvas background'),
    ('sheet_column_bg_colour',          'Sheet column background'),
    ('sheet_cursor_view_line_colour',   'Sheet navigating cursor line'),
    ('sheet_cursor_edit_line_colour',   'Sheet editing cursor line'),
    ('sheet_grid_level_1_colour',       'Sheet grid level 1'),
    ('sheet_grid_level_2_colour',       'Sheet grid level 2'),
    ('sheet_grid_level_3_colour',       'Sheet grid level 3'),
    ('sheet_header_bg_colour',          'Sheet header background'),
    ('sheet_header_fg_colour',          'Sheet header text'),
    ('sheet_header_border_colour',      'Sheet header border'),
    ('sheet_ruler_bg_colour',           'Sheet ruler background'),
    ('sheet_ruler_fg_colour',           'Sheet ruler foreground'),
    ('sheet_trigger_default_colour',    'Default trigger'),
    ('sheet_trigger_note_on_colour',    'Note on trigger'),
    ('sheet_trigger_hit_colour',        'Hit trigger'),
    ('sheet_trigger_note_off_colour',   'Note off trigger'),
    ('sheet_trigger_warning_bg_colour', 'Trigger warning background'),
    ('sheet_trigger_warning_fg_colour', 'Trigger warning text'),
]

_COLOUR_DESCS_DICT = dict(_COLOUR_DESCS)


class ColoursModel(QAbstractItemModel):

    def __init__(self):
        super().__init__()
        self._ui_model = None

        self._colours = []

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model

        self._make_colour_list()

    def _make_colour_list(self):
        style_manager = self._ui_model.get_style_manager()

        colours = []

        sheet_colours = None

        for k, _ in _COLOUR_DESCS:
            colour = style_manager.get_style_param(k)
            if k.startswith('sheet_'):
                if not sheet_colours:
                    sheet_colours = ColourCategoryModel('Sheet')
                    colours.append(sheet_colours)
                sheet_colours.add_colour(ColourModel(k, colour, sheet_colours))
            else:
                colours.append(ColourModel(k, colour))

        self._colours = colours

    def get_colour_indices(self):
        for row, node in enumerate(self._colours):
            if isinstance(node, ColourCategoryModel):
                for i in range(node.get_colour_count()):
                    contained_node = node.get_colour(i)
                    yield self.createIndex(i, 1, contained_node)
            else:
                yield self.createIndex(row, 1, node)

    # Qt interface

    def columnCount(self, _):
        return 2

    def rowCount(self, parent):
        if not parent.isValid():
            return len(self._colours)
        node = parent.internalPointer()
        if isinstance(node, ColourCategoryModel):
            return node.get_colour_count()
        return 0

    def index(self, row, col, parent):
        if not parent.isValid():
            node = self._colours[row]
        else:
            parent_node = parent.internalPointer()
            if isinstance(parent_node, ColourCategoryModel):
                node = parent_node.get_colour(row)
            else:
                return QModelIndex()
        return self.createIndex(row, col, node)

    def parent(self, index):
        if not index.isValid():
            return QModelIndex()
        node = index.internalPointer()
        if isinstance(node, ColourCategoryModel):
            return QModelIndex()
        elif isinstance(node, ColourModel):
            category = node.get_category()
            if category:
                index = category.get_index_of_colour(node)
                return self.createIndex(index, 0, category)
            else:
                return QModelIndex()
        else:
            assert False

    def data(self, index, role):
        if role == Qt.DisplayRole:
            assert index.isValid()
            node = index.internalPointer()
            column = index.column()
            if isinstance(node, ColourCategoryModel):
                if column == 0:
                    return node.get_name()
                elif column == 1:
                    return ''
            elif isinstance(node, ColourModel):
                if column == 0:
                    key = node.get_key()
                    desc = _COLOUR_DESCS_DICT[key]
                    category = node.get_category()
                    if category:
                        strip_prefix = category.get_name() + ' '
                        if desc.startswith(strip_prefix):
                            desc = desc[len(strip_prefix):].capitalize()
                    return desc
                elif column == 1:
                    return node.get_colour()
            else:
                assert False

    def flags(self, index):
        default_flags = super().flags(index)
        if not index.isValid():
            return default_flags
        return Qt.ItemIsEnabled


class ColourButton(QPushButton):

    colourSelected = Signal(str, name='colourSelected')

    def __init__(self, key):
        super().__init__()
        self._key = key

        self.setFixedSize(QSize(48, 16))
        self.setAutoFillBackground(True)

        QObject.connect(self, SIGNAL('clicked()'), self._clicked)

    def get_key(self):
        return self._key

    def set_colour(self, colour):
        style = 'QPushButton {{ background-color: {}; }}'.format(colour)
        self.setStyleSheet(style)

    def _clicked(self):
        QObject.emit(self, SIGNAL('colourSelected(QString)'), self._key)


class ColourChooser(QWidget):

    colourChanged = Signal('colourChanged()')

    _STATE_IDLE = 'idle'
    _STATE_EDITING_HUE = 'editing_hue'
    _STATE_EDITING_SV = 'editing_sv'

    def __init__(self):
        super().__init__()

        self._config = {
            'hue_ring_thickness'  : 0.25,
            'sv_gradient_radius'  : 16,
            'hue_marker_thickness': 2.5,
            'sv_marker_radius'    : 6,
            'sv_marker_thickness' : 2,
        }

        self._hue_outer_radius = None
        self._hue_ring = None
        self._sv_triangle = None
        self._sv_gradients = []

        self._colour = QColor(0, 0, 0)
        self._hue = 0
        self._saturation = 0
        self._value = 0

        self._make_sv_gradients()

        self._state = self._STATE_IDLE

        self.setMouseTracking(True)

    def set_colour(self, colour):
        if self._colour != colour:
            self._colour = colour
            self._hue, self._saturation, self._value, _ = self._colour.getHsvF()
            self._sv_triangle = None
            self.update()

    def get_colour(self):
        return self._colour

    def _set_colour(self, colour):
        if self._colour != colour:
            self._colour = colour
            self._sv_triangle = None
            QObject.emit(self, SIGNAL('colourChanged()'))
            self.update()

    def _get_rel_coords(self, event):
        x = event.x() - (self.width() // 2)
        y = event.y() - (self.height() // 2)
        return x, y

    def _length_and_dir(self, v):
        x = v.x()
        y = v.y()
        vlen = math.sqrt(x * x + y * y)
        vdir = QPointF(1, 0)
        if vlen > 0.05:
            vdir = v * (1 / vlen)
        return vlen, vdir

    def _set_hue_from_coords(self, x, y):
        v = QPointF(x, y)
        _, vdir = self._length_and_dir(v)

        def dot(a, b):
            return a.x() * b.x() + a.y() * b.y()

        d = dot(vdir, QPointF(1, 0))
        angle = math.acos(min(max(-1, d), 1))
        if vdir.y() > 0:
            angle = 2 * math.pi - angle

        new_hue = angle / (2 * math.pi)
        self._hue = new_hue
        new_colour = QColor.fromHsvF(new_hue, self._saturation, self._value)
        self._set_colour(new_colour)

    def mouseMoveEvent(self, event):
        x, y = self._get_rel_coords(event)

        if self._state == self._STATE_IDLE:
            pass
        elif self._state == self._STATE_EDITING_HUE:
            self._set_hue_from_coords(x, y)
        elif self._state == self._STATE_EDITING_SV:
            pass
        else:
            assert False

    def mousePressEvent(self, event):
        if event.buttons() != Qt.LeftButton:
            return

        x, y = self._get_rel_coords(event)

        hue_ring_thickness = self._config['hue_ring_thickness']
        hue_inner_radius = self._hue_outer_radius * (1 - hue_ring_thickness)

        dist_from_center = math.sqrt(x * x + y * y)
        if hue_inner_radius < dist_from_center < self._hue_outer_radius:
            self._state = self._STATE_EDITING_HUE
            self._set_hue_from_coords(x, y)
            return

    def mouseReleaseEvent(self, event):
        self._state = self._STATE_IDLE

    def _get_marker_colour(self, colour):
        intensity = colour.red() * 0.212 + colour.green() * 0.715 + colour.blue() * 0.072
        return QColor(0xff, 0xff, 0xff) if intensity < 127 else QColor(0, 0, 0)

    def _make_sv_gradients(self):
        radius = self._config['sv_gradient_radius']
        wh = radius * 2

        red = QImage(wh, wh, QImage.Format_ARGB32_Premultiplied)
        red.fill(0)

        # Make black -> white gradient
        painter = QPainter(red)
        painter.translate(radius, radius)
        painter.setPen(Qt.NoPen)

        white_grad = QLinearGradient(
                radius * math.cos(math.pi * 2 / 3), radius * math.sin(math.pi * 2 / 3),
                radius * (math.cos(math.pi * 4 / 3) + 1) / 2,
                radius * math.sin(math.pi * 4 / 3) / 2)
        white_grad.setColorAt(0, QColor(0xff, 0xff, 0xff))
        white_grad.setColorAt(1, QColor(0, 0, 0))
        painter.fillRect(-radius, -radius, wh, wh, QBrush(white_grad))
        painter.end()

        # Use the black -> white gradient as a basis for the other images
        yellow = QImage(red)
        green = QImage(red)
        magenta = QImage(red)

        # Add colour gradients with additive blending
        def fill_one_component_column(image, x, start_y, end_y, alpha_add, shift):
            mask = 0xffffffff ^ (0xff << shift)
            for y in range(start_y, end_y):
                argb = image.pixel(x, y)
                c = (argb >> shift) & 0xff
                c = min(0xff, c + alpha_add)
                argb = (argb & mask) + (c << shift)
                image.setPixel(x, y, argb)

        def fill_two_component_column(
                image, x, start_y, end_y, alpha_add, shift1, shift2):
            mask = 0xffffffff ^ (0xff << shift1) ^ (0xff << shift2)
            for y in range(start_y, end_y):
                argb = image.pixel(x, y)
                c1 = (argb >> shift1) & 0xff
                c2 = (argb >> shift2) & 0xff
                c1 = min(0xff, c1 + alpha_add)
                c2 = min(0xff, c2 + alpha_add)
                argb = (argb & mask) + (c1 << shift1) + (c2 << shift2)
                image.setPixel(x, y, argb)

        start_x = int(radius + radius * math.cos(math.pi * 2 / 3))
        end_x = wh
        col_count = end_x - start_x
        for x in range(start_x, end_x):
            col_index = (x - start_x)
            alpha_add_norm = min(max(0, col_index / col_count), 1)
            alpha_add = int(alpha_add_norm * 0xff)
            start_y = col_index // 2
            end_y = wh - (col_index // 2)

            fill_one_component_column(red, x, start_y, end_y, alpha_add, 16)
            fill_one_component_column(green, x, start_y, end_y, alpha_add, 8)
            fill_two_component_column(yellow, x, start_y, end_y, alpha_add, 16, 8)
            fill_two_component_column(magenta, x, start_y, end_y, alpha_add, 16, 0)

        # Create missing gradients from existing ones
        cyan = yellow.rgbSwapped()
        blue = red.rgbSwapped()

        self._sv_gradients = [red, yellow, green, cyan, blue, magenta]

    def _clear_cache(self):
        self._hue_ring = None
        self._sv_triangle = None

    def paintEvent(self, event):
        width = self.width()
        height = self.height()

        hue_outer_radius = min(width, height) // 2
        if hue_outer_radius < 1:
            return

        hue_inner_radius = hue_outer_radius * (1 - self._config['hue_ring_thickness'])

        if hue_outer_radius != self._hue_outer_radius:
            self._hue_outer_radius = hue_outer_radius
            self._clear_cache()

        painter = QPainter(self)
        painter.translate(width // 2, height // 2)
        painter.setRenderHint(QPainter.SmoothPixmapTransform)

        hue_ring = self._get_hue_ring()
        sv_triangle = self._get_sv_triangle(hue_inner_radius)

        painter.drawImage(-hue_inner_radius, -hue_inner_radius, sv_triangle)
        painter.drawImage(-hue_outer_radius, -hue_outer_radius, hue_ring)

        # Draw markers
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setBrush(Qt.NoBrush)

        # Hue marker
        hue_colour = QColor.fromHsvF(self._hue, 1, 1)
        hue_marker_colour = self._get_marker_colour(hue_colour)
        hue_angle = self._hue * 2 * math.pi
        hue_dir = QPointF(math.cos(hue_angle), -math.sin(hue_angle))
        hue_pen = QPen()
        hue_pen.setColor(hue_marker_colour)
        hue_pen.setWidth(self._config['hue_marker_thickness'])
        painter.setPen(hue_pen)
        painter.drawLine(hue_dir * hue_inner_radius, hue_dir * hue_outer_radius)

        # SV marker
        sv_marker_colour = self._get_marker_colour(self._colour)
        sv_marker_radius = self._config['sv_marker_radius']

        sv_black_dir = QPointF(
                math.cos(hue_angle + math.pi * 2 / 3),
                -math.sin(hue_angle + math.pi * 2 / 3))
        sv_white_dir = QPointF(
                math.cos(hue_angle - math.pi * 2 / 3),
                -math.sin(hue_angle - math.pi * 2 / 3))
        sv_colour_pos = hue_dir * hue_inner_radius
        sv_black_pos = sv_black_dir * hue_inner_radius
        sv_white_pos = sv_white_dir * hue_inner_radius

        sv_val_start_pos = utils.lerp_val(sv_black_pos, sv_white_pos, self._value)
        sat_max_length, sat_dir = self._length_and_dir(sv_colour_pos - sv_white_pos)
        sat_length = sat_max_length * self._value
        sv_final_pos = sv_val_start_pos + (sat_dir * (sat_length * self._saturation))

        sv_pen = QPen()
        sv_pen.setColor(sv_marker_colour)
        sv_pen.setWidth(self._config['sv_marker_thickness'])
        painter.setPen(sv_pen)
        painter.drawEllipse(sv_final_pos, sv_marker_radius, sv_marker_radius)

    def _get_hue_ring(self):
        if self._hue_ring:
            return self._hue_ring

        diam = self._hue_outer_radius * 2
        cut_diam = diam * (1 - self._config['hue_ring_thickness'])
        cut_offset = (diam - cut_diam) / 2

        self._hue_ring = QImage(diam, diam, QImage.Format_ARGB32_Premultiplied)
        self._hue_ring.fill(0)

        hues = QConicalGradient()
        hues.setCenter(QPointF(self._hue_outer_radius, self._hue_outer_radius))
        hues.setColorAt(0,     QColor(0xff, 0,    0))
        hues.setColorAt(1 / 6, QColor(0xff, 0xff, 0))
        hues.setColorAt(2 / 6, QColor(0,    0xff, 0))
        hues.setColorAt(3 / 6, QColor(0,    0xff, 0xff))
        hues.setColorAt(4 / 6, QColor(0,    0,    0xff))
        hues.setColorAt(5 / 6, QColor(0xff, 0,    0xff))
        hues.setColorAt(1,     QColor(0xff, 0,    0))

        painter = QPainter(self._hue_ring)
        painter.setPen(Qt.NoPen)
        painter.setRenderHint(QPainter.Antialiasing)

        painter.setBrush(QBrush(hues))
        painter.drawEllipse(QRectF(0, 0, diam, diam))

        painter.setBrush(QColor(0xff, 0xff, 0xff))
        painter.setCompositionMode(QPainter.CompositionMode_DestinationOut)
        painter.drawEllipse(QRectF(cut_offset, cut_offset, cut_diam, cut_diam))

        return self._hue_ring

    def _get_sv_triangle(self, inner_radius):
        if self._sv_triangle:
            return self._sv_triangle

        # Get hue gradient
        hue_scaled = self._hue * 6
        hue_index1 = int(math.floor(hue_scaled)) % 6
        hue_index2 = (hue_index1 + 1) % 6
        hue2_alpha = hue_scaled - math.floor(hue_scaled)

        hue_grad = QImage(self._sv_gradients[hue_index1])
        hue2_grad = QImage(self._sv_gradients[hue_index2])

        hue_painter = QPainter(hue_grad)
        hue_painter.setOpacity(hue2_alpha)
        hue_painter.drawImage(0, 0, hue2_grad)
        hue_painter.end()

        # Draw the triangle
        wh = inner_radius * 2
        self._sv_triangle = QImage(wh, wh, QImage.Format_ARGB32)
        self._sv_triangle.fill(0)

        painter = QPainter(self._sv_triangle)
        painter.setRenderHint(QPainter.Antialiasing)
        painter.setRenderHint(QPainter.SmoothPixmapTransform)
        painter.translate(inner_radius, inner_radius)
        painter.rotate(-self._hue * 360)
        painter.setPen(Qt.NoPen)

        ir = inner_radius

        painter.drawImage(QRectF(-ir, -ir, wh, wh), hue_grad)

        triangle = QPolygonF([
            QPointF(ir, 0),
            QPointF(ir * math.cos(math.pi * 2 / 3), ir * math.sin(math.pi * 2 / 3)),
            QPointF(ir * math.cos(math.pi * 4 / 3), ir * math.sin(math.pi * 4 / 3))])
        mask = QPolygonF(QRectF(-ir - 1, -ir - 1, wh + 2, wh + 2)).subtracted(triangle)
        painter.setBrush(QColor(0, 0, 0))
        painter.setCompositionMode(QPainter.CompositionMode_DestinationOut)
        painter.drawPolygon(mask)

        return self._sv_triangle

    def minimumSizeHint(self):
        return QSize(192, 192)


class ColourCodeValidator(QValidator):

    def __init__(self):
        super().__init__()

    def validate(self, contents, pos):
        if (not contents) or (contents[0] != '#') or (len(contents) > 7):
            return (QValidator.Invalid, contents, pos)
        if not all(c in string.hexdigits for c in contents[1:]):
            return (QValidator.Invalid, contents, pos)
        if len(contents) < 7:
            return (QValidator.Intermediate, contents, pos)
        return (QValidator.Acceptable, contents, pos)


class ColourComparison(QWidget):

    def __init__(self):
        super().__init__()
        self._orig_colour = QColor(0, 0, 0)
        self._new_colour = QColor(0, 0, 0)

        self.setAutoFillBackground(False)
        self.setAttribute(Qt.WA_OpaquePaintEvent)
        self.setAttribute(Qt.WA_NoSystemBackground)

    def set_original_colour(self, colour):
        self._orig_colour = colour
        self.update()

    def set_new_colour(self, colour):
        self._new_colour = colour
        self.update()

    def paintEvent(self, event):
        width = self.width()
        height = self.height()

        painter = QPainter(self)
        painter.fillRect(0, 0, width // 2, height, self._orig_colour)
        painter.fillRect(width // 2, 0, width // 2, height, self._new_colour)

    def minimumSizeHint(self):
        return QSize(128, 32)


class ColourEditor(QWidget):

    colourModified = Signal(str, str, 'colourModified')

    def __init__(self):
        super().__init__()
        self._key = None
        self._orig_colour = None
        self._new_colour = None

        self._chooser = ColourChooser()

        self._code_editor = QLineEdit()
        self._code_editor.setValidator(ColourCodeValidator())

        self._comparison = ColourComparison()

        self._revert_button = QPushButton('Revert')
        self._accept_button = QPushButton('Done')

        # Code editor layout
        cl = QHBoxLayout()
        cl.setContentsMargins(0, 0, 0, 0)
        cl.setSpacing(2)
        cl.addWidget(QLabel('Code:'))
        cl.addWidget(self._code_editor, 1)

        # Colour comparison layout
        orig_label = QLabel('Original')
        orig_label.setAlignment(Qt.AlignHCenter)
        orig_label.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Maximum)
        new_label = QLabel('New')
        new_label.setAlignment(Qt.AlignHCenter)
        new_label.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Maximum)

        ctl = QHBoxLayout()
        ctl.setContentsMargins(0, 0, 0, 0)
        ctl.setSpacing(2)
        ctl.addWidget(orig_label)
        ctl.addWidget(new_label)
        compl = QVBoxLayout()
        compl.setContentsMargins(0, 0, 0, 0)
        compl.setSpacing(2)
        compl.addLayout(ctl)
        compl.addWidget(self._comparison)

        # Button layout
        bl = QHBoxLayout()
        bl.setContentsMargins(0, 0, 0, 0)
        bl.setSpacing(2)
        bl.addWidget(self._revert_button)
        bl.addWidget(self._accept_button)

        # Top layout
        v = QVBoxLayout()
        v.setContentsMargins(4, 4, 4, 4)
        v.setSpacing(4)
        v.addWidget(self._chooser)
        v.addLayout(cl)
        v.addLayout(compl)
        v.addLayout(bl)
        self.setLayout(v)

        QObject.connect(self._chooser, SIGNAL('colourChanged()'), self._change_colour)

        QObject.connect(
                self._code_editor,
                SIGNAL('textEdited(const QString&)'),
                self._change_code)

        QObject.connect(self._revert_button, SIGNAL('clicked()'), self._revert)

        QObject.connect(self._accept_button, SIGNAL('clicked()'), self.hide)

    def set_colour(self, key, colour):
        self._key = key
        self._orig_colour = colour
        self._new_colour = colour

        desc = _COLOUR_DESCS_DICT[key]
        if not desc.startswith(('Note on ', 'Note off ', 'Hit ')):
            desc = desc[0].lower() + desc[1:]
        self.setWindowTitle('Colour of ' + desc)

        self._chooser.set_colour(self._orig_colour)

        code = utils.get_str_from_colour(self._orig_colour)
        old_block = self._code_editor.blockSignals(True)
        self._code_editor.setText(code)
        self._code_editor.blockSignals(old_block)

        self._comparison.set_original_colour(self._orig_colour)
        self._comparison.set_new_colour(self._new_colour)

        self._revert_button.setEnabled(False)

    def _update_colour(self, new_colour):
        self._new_colour = new_colour
        self._chooser.set_colour(self._new_colour)
        self._comparison.set_new_colour(self._new_colour)
        self._revert_button.setEnabled(self._new_colour != self._orig_colour)

    def _change_colour(self):
        colour = self._chooser.get_colour()
        self._update_colour(colour)
        text = utils.get_str_from_colour(colour)
        QObject.emit(self, SIGNAL('colourModified(QString, QString)'), self._key, text)

    def _change_code(self, text):
        if self._code_editor.hasAcceptableInput():
            self._update_colour(utils.get_colour_from_str(text))
            QObject.emit(
                    self, SIGNAL('colourModified(QString, QString)'), self._key, text)

    def _revert(self):
        orig_code = utils.get_str_from_colour(self._orig_colour)
        QObject.emit(
                self, SIGNAL('colourModified(QString, QString)'), self._key, orig_code)
        self.hide()

    def sizeHint(self):
        return QSize(256, 64)


class Colours(QTreeView):

    def __init__(self):
        super().__init__()
        self._ui_model = None
        self._updater = None

        self._model = None

        self._colour_editor = ColourEditor()

        header = self.header()
        header.setStretchLastSection(False)
        header.setResizeMode(0, QHeaderView.Stretch)
        self.setHeaderHidden(True)

    def set_ui_model(self, ui_model):
        self._ui_model = ui_model
        self._updater = ui_model.get_updater()
        self._updater.register_updater(self._perform_updates)

        self._model = ColoursModel()
        self._model.set_ui_model(ui_model)
        self.setModel(self._model)

        for index in self._model.get_colour_indices():
            node = index.internalPointer()
            if isinstance(node, ColourModel):
                key = node.get_key()
                button = ColourButton(key)
                QObject.connect(
                        button,
                        SIGNAL('colourSelected(QString)'),
                        self._open_colour_editor)
                self.setIndexWidget(index, button)

        self.expandAll()

        self.resizeColumnToContents(0)
        self.setColumnWidth(1, 48)

        QObject.connect(
                self._colour_editor,
                SIGNAL('colourModified(QString, QString)'),
                self._update_colour)

        self._update_enabled()
        self._update_button_colours()

    def unregister_updaters(self):
        self._updater.unregister_updater(self._perform_updates)

    def _perform_updates(self, signals):
        if 'signal_style_changed' in signals:
            self._update_enabled()
            self._update_button_colours()

    def _update_enabled(self):
        style_manager = self._ui_model.get_style_manager()
        self.setEnabled(style_manager.is_custom_style_enabled())

    def _update_button_colours(self):
        style_manager = self._ui_model.get_style_manager()

        for index in self._model.get_colour_indices():
            button = self.indexWidget(index)
            if button:
                key = button.get_key()
                colour = style_manager.get_style_param(key)
                button.set_colour(colour)

    def _open_colour_editor(self, key):
        style_manager = self._ui_model.get_style_manager()
        cur_colour_str = style_manager.get_style_param(key)
        cur_colour = utils.get_colour_from_str(cur_colour_str)
        self._colour_editor.set_colour(key, cur_colour)
        self._colour_editor.show()

    def _update_colour(self, key, colour_code):
        style_manager = self._ui_model.get_style_manager()
        style_manager.set_style_param(key, colour_code)
        self._updater.signal_update(set(['signal_style_changed']))

    def hideEvent(self, event):
        self._colour_editor.hide()


